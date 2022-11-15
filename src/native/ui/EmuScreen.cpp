
#include <algorithm>
#include "base/display.h"
#include "base/logging.h"
#include "base/timeutil.h"
#include "gfx_es2/glsl_program.h"
#include "gfx_es2/gl_state.h"
#include "gfx_es2/draw_text.h"
#include "gfx_es2/fbo.h"
#include "input/input_state.h"
#include "main.h"
#include "ui/ui.h"
#include "ui/ui_context.h"
#include "i18n/i18n.h"
#include "Common/KeyMap.h"
#include "Core/Config.h"
#include "Core/CoreTiming.h"
#include "Core/CoreParameter.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "Core/Reporting.h"
#include "Core/System.h"
#include "GPU/GPUState.h"
#include "GPU/GPUInterface.h"
#include "GPU/GLES/Framebuffer.h"
#include "Core/HLE/sceCtrl.h"
#include "Core/HLE/sceDisplay.h"
#include "Core/Debugger/SymbolMap.h"
#include "Core/SaveState.h"
#include "Core/MIPS/MIPS.h"
#include "Core/HLE/__sceAudio.h"
#include "EmuScreen.h"



EmuScreen::EmuScreen(const std::string &filename)
    : bootPending_(true), gamePath_(filename), invalid_(true), quit_(false), pauseTrigger_(false),
      saveStatePreviewShownTime_(0.0), saveStatePreview_(nullptr)
{
    memset(axisState_, 0, sizeof(axisState_));
}
EmuScreen::~EmuScreen()
{
    if (!invalid_) {
        PSP_Shutdown();
    }
}
void EmuScreen::bootGame(const std::string &filename)
{
    if (PSP_IsIniting()) {
        std::string error_string;
        bootPending_ = !PSP_InitUpdate(&error_string);
        if (!bootPending_) {
            invalid_ = !PSP_IsInited();
            if (invalid_) {
                errorMessage_ = error_string;
                ERROR_LOG(BOOT, "%s", errorMessage_.c_str());
                System_SendMessage("event", "failstartgame");
                return;
            }
            bootComplete();
        }
        return;
    }
    invalid_ = true;
    CoreParameter coreParam;
    coreParam.cpuCore = g_Config.bJit ? CPU_JIT : CPU_INTERPRETER;
    coreParam.gpuCore = g_Config.bSoftwareRendering ? GPU_SOFTWARE : GPU_GLES;
    if (g_Config.iGPUBackend == GPU_BACKEND_DIRECT3D9) {
        coreParam.gpuCore = GPU_DIRECTX9;
    }
    coreParam.enableSound  = g_Config.bEnableSound;
    coreParam.fileToStart  = filename;
    coreParam.mountIso     = "";
    coreParam.mountRoot    = "";
    coreParam.startPaused  = false;
    coreParam.printfEmuLog = false;
    coreParam.headLess     = false;
    const Bounds &bounds   = screenManager()->getUIContext()->GetBounds();
    if (g_Config.iInternalResolution == 0) {
        coreParam.renderWidth  = bounds.w;
        coreParam.renderHeight = bounds.h;
    } else {
        if (g_Config.iInternalResolution < 0)
            g_Config.iInternalResolution = 1;
        coreParam.renderWidth  = 480 * g_Config.iInternalResolution;
        coreParam.renderHeight = 272 * g_Config.iInternalResolution;
    }
    std::string error_string;
    if (!PSP_InitStart(coreParam, &error_string)) {
        bootPending_  = false;
        invalid_      = true;
        errorMessage_ = error_string;
        ERROR_LOG(BOOT, "%s", errorMessage_.c_str());
        System_SendMessage("event", "failstartgame");
    }
}
void EmuScreen::bootComplete()
{
    UpdateUIState(UISTATE_INGAME);
    NOTICE_LOG(BOOT, "Loading %s...", PSP_CoreParameter().fileToStart.c_str());
    autoLoad();
    memset(virtKeys, 0, sizeof(virtKeys));

    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        const char *renderer = (const char *)glGetString(GL_RENDERER);
        if (strstr(renderer, "Chainfire3D") != 0) {
        } else if (strstr(renderer, "GLTools") != 0) {
        }
    }
    System_SendMessage("event", "startgame");
}
void EmuScreen::dialogFinished(const Screen *dialog, DialogResult result)
{
    if (result == DR_OK || quit_) {
        quit_ = false;
    }
    // RecreateViews();
}
static void AfterStateLoad(bool success, void *ignored)
{
    Core_EnableStepping(false);
}
void EmuScreen::sendMessage(const char *message, const char *value)
{
    if (!strcmp(message, "stop")) {
        PSP_Shutdown();
        bootPending_ = false;
        invalid_     = true;
    } else if (!strcmp(message, "boot")) {
        const char *ext = strrchr(value, '.');
        if (!strcmp(ext, ".ppst")) {
            SaveState::Load(value, &AfterStateLoad);
        } else {
            PSP_Shutdown();
            bootPending_ = true;
            bootGame(value);
        }
    } else if (!strcmp(message, "window minimized")) {
        if (!strcmp(value, "true")) {
            gstate_c.skipDrawReason |= SKIPDRAW_WINDOW_MINIMIZED;
        } else {
            gstate_c.skipDrawReason &= ~SKIPDRAW_WINDOW_MINIMIZED;
        }
    }
}
inline float clamp1(float x)
{
    if (x > 1.0f)
        return 1.0f;
    if (x < -1.0f)
        return -1.0f;
    return x;
}
bool EmuScreen::touch(const TouchInput &touch)
{
    if (root_) {
        root_->Touch(touch);
        return true;
    } else {
        return false;
    }
}
bool EmuScreen::key(const KeyInput &key)
{
    std::vector<int> pspKeys;
    KeyMap::KeyToPspButton(key.deviceId, key.keyCode, &pspKeys);
    if (pspKeys.size() && (key.flags & KEY_IS_REPEAT)) {
        return true;
    }
    for (size_t i = 0; i < pspKeys.size(); i++) {
        pspKey(pspKeys[i], key.flags);
    }
    if (!pspKeys.size() || key.deviceId == DEVICE_ID_DEFAULT) {
        if ((key.flags & KEY_DOWN) && key.keyCode == NKCODE_BACK) {
            pauseTrigger_ = true;
            return true;
        }
    }
    return pspKeys.size() > 0;
}
void EmuScreen::pspKey(int pspKeyCode, int flags)
{
    if (flags & KEY_DOWN)
        __CtrlButtonDown(pspKeyCode);
    if (flags & KEY_UP)
        __CtrlButtonUp(pspKeyCode);
}
bool EmuScreen::axis(const AxisInput &axis)
{
    return false;
}
inline bool IsAnalogStickKey(int key)
{
    switch (key) {
        case VIRTKEY_AXIS_X_MIN:
        case VIRTKEY_AXIS_X_MAX:
        case VIRTKEY_AXIS_Y_MIN:
        case VIRTKEY_AXIS_Y_MAX:
        case VIRTKEY_AXIS_RIGHT_X_MIN:
        case VIRTKEY_AXIS_RIGHT_X_MAX:
        case VIRTKEY_AXIS_RIGHT_Y_MIN:
        case VIRTKEY_AXIS_RIGHT_Y_MAX:
            return true;
        default:
            return false;
    }
}
void EmuScreen::update(InputState &input)
{
    if (bootPending_)
        bootGame(gamePath_);

    UIScreen::update(input);
    const Bounds &bounds = screenManager()->getUIContext()->GetBounds();

    PSP_CoreParameter().pixelWidth  = pixel_xres * bounds.w / dp_xres;
    PSP_CoreParameter().pixelHeight = pixel_yres * bounds.h / dp_yres;
}
void EmuScreen::checkPowerDown()
{
    if (coreState == CORE_POWERDOWN && !PSP_IsIniting()) {
        if (PSP_IsInited()) {
            PSP_Shutdown();
        }
        ILOG("SELF-POWERDOWN!");
        bootPending_ = false;
        invalid_     = true;
    }
}
void EmuScreen::render()
{
    if (invalid_) {
        checkPowerDown();
        return;
    }
    if (PSP_CoreParameter().freezeNext) {
        PSP_CoreParameter().frozen     = true;
        PSP_CoreParameter().freezeNext = false;
        SaveState::SaveToRam(freezeState_);
    } else if (PSP_CoreParameter().frozen) {
        if (CChunkFileReader::ERROR_NONE != SaveState::LoadFromRam(freezeState_)) {
            ERROR_LOG(HLE, "Failed to load freeze state. Unfreezing.");
            PSP_CoreParameter().frozen = false;
        }
    }
    ReapplyGfxState();
    int blockTicks = usToCycles(1000000 / 10);
    while (coreState == CORE_RUNNING) {
        PSP_RunLoopFor(blockTicks);
    }
    if (coreState == CORE_NEXTFRAME) {
        coreState = CORE_RUNNING;
    }
    checkPowerDown();
    if (invalid_)
        return;
    bool useBufferedRendering = g_Config.iRenderingMode != FB_NON_BUFFERED_MODE;
    if (useBufferedRendering && g_Config.iGPUBackend == GPU_BACKEND_OPENGL)
        fbo_unbind();
    if (g_Config.bShowDebugStats || g_Config.iShowFPSCounter || g_Config.bShowTouchControls ||
        g_Config.bShowDeveloperMenu || g_Config.bShowAudioDebug) {
        Thin3DContext *thin3d = screenManager()->getThin3DContext();
        screenManager()->getUIContext()->Begin();
        T3DViewport viewport;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width    = pixel_xres;
        viewport.Height   = pixel_yres;
        viewport.MaxDepth = 1.0;
        viewport.MinDepth = 0.0;
        thin3d->SetViewports(1, &viewport);
        DrawBuffer *draw2d = screenManager()->getUIContext()->Draw();
        if (root_) {
            UI::LayoutViewHierarchy(*screenManager()->getUIContext(), root_);
            root_->Draw(*screenManager()->getUIContext());
        }

        screenManager()->getUIContext()->End();
    }
}
void EmuScreen::deviceLost()
{
    ILOG("EmuScreen::deviceLost()");
    if (gpu)
        gpu->DeviceLost();
    // RecreateViews();
}
void EmuScreen::autoLoad()
{
    int lastSlot = SaveState::GetNewestSlot();
    if (g_Config.bEnableAutoLoad && lastSlot != -1) {
        SaveState::LoadSlot(lastSlot, SaveState::Callback(), 0);
        g_Config.iCurrentStateSlot = lastSlot;
    }
}
