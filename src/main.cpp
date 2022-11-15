#include <locale.h>
#include <algorithm>
#include <memory>
#include "Core/Loaders.h"
#include "base/display.h"
#include "base/logging.h"
#include "base/mutex.h"
#include "main.h"
#include "file/vfs.h"
#include "file/zip_read.h"
#include "thread/thread.h"
#include "gfx_es2/gl_state.h"
#include "gfx_es2/draw_text.h"
#include "gfx/gl_lost_manager.h"
#include "gfx/texture.h"
#include "i18n/i18n.h"
#include "input/input_state.h"
#include "math/fast/fast_math.h"
#include "math/math_util.h"
#include "math/lin/matrix4x4.h"
#include "thin3d/thin3d.h"

#include "ui/ui.h"
#include "ui/screen.h"
#include "ui/ui_context.h"
#include "ui/view.h"
#include "ui/EmuScreen.h"

#include "util/text/utf8.h"
#include "Common/CPUDetect.h"
#include "Common/FileUtil.h"
#include "Common/LogManager.h"
#include "Common/MemArena.h"
#include "Core/Config.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "Core/SaveState.h"
#include "Core/Screenshot.h"
#include "Core/System.h"
#include "Core/HLE/__sceAudio.h"
#include "Core/HLE/sceCtrl.h"
// #include "Core/Util/GameManager.h"
#include "Core/Util/AudioFormat.h"
#include "Common/KeyMap.h"

#include <unistd.h>
#include <pwd.h>

#ifdef RPI
#include <bcm_host.h>
#endif

#include <algorithm>
#include "base/display.h"
#include "base/logging.h"
#include "base/timeutil.h"
#include "gfx_es2/gl_state.h"
#include "input/input_state.h"
#include "input/keycodes.h"

#include "util/text/utf8.h"
#include "math/math_util.h"

#include "Core/System.h"
#include "Core/Core.h"
#include "Core/Config.h"

const char *PPSSPP_GIT_VERSION = "6a64244";



static UI::Theme ui_theme;
Thin3DTexture   *uiTexture;
ScreenManager   *screenManager;
std::string      config_filename;
std::thread     *graphicsLoadThread;
std::string      boot_filename = "";

static bool isOuya;
static bool resized = false;

struct PendingMessage
{
    std::string msg;
    std::string value;
};

static recursive_mutex             pendingMutex;
static std::vector<PendingMessage> pendingMessages;
static Thin3DContext              *thin3d;
static UIContext                  *uiContext;

Thin3DContext *GetThin3D()
{
    return thin3d;
}


GlobalUIState lastUIState = UISTATE_MENU;
GlobalUIState GetUIState();

static SDL_Window *g_Screen                    = NULL;
static bool        g_ToggleFullScreenNextFrame = false;
static int         g_QuitRequested             = 0;
static int         g_DesktopWidth              = 0;
static int         g_DesktopHeight             = 0;


InputState input_state;

static const int legacyKeyMap[]{
    NKCODE_X, NKCODE_S,      NKCODE_Z,       NKCODE_A,         NKCODE_W,         NKCODE_Q,
    NKCODE_1, NKCODE_2,      NKCODE_DPAD_UP, NKCODE_DPAD_DOWN, NKCODE_DPAD_LEFT, NKCODE_DPAD_RIGHT,
    0,        NKCODE_ESCAPE, NKCODE_I,       NKCODE_K,         NKCODE_J,         NKCODE_L,
};





int NativeMix(short *audio, int num_samples)
{
    int sample_rate = System_GetPropertyInt(SYSPROP_AUDIO_SAMPLE_RATE);
    num_samples     = __AudioMix(audio, num_samples, sample_rate > 0 ? sample_rate : 44100);
    return num_samples;
}
void NativeInit(int argc, const char *argv[], const char *savegame_directory, const char *external_directory,
                const char *installID, bool fs)
{
    InitFastMath(cpu_info.bNEON);
    SetupAudioFormats();
    FPU_SetFastMode();
    bool skipLogo = false;
    setlocale(LC_ALL, "C");
    std::string user_data_path = savegame_directory;
    pendingMessages.clear();
    VFSRegister("", new DirectoryAssetReader((File::GetExeDirectory() + "assets/").c_str()));
    VFSRegister("", new DirectoryAssetReader((File::GetExeDirectory()).c_str()));
    VFSRegister("", new DirectoryAssetReader("/usr/share/ppsspp/assets/"));
    VFSRegister("", new DirectoryAssetReader(savegame_directory));
    std::string config;
    if (getenv("XDG_CONFIG_HOME") != NULL)
        config = getenv("XDG_CONFIG_HOME");
    else if (getenv("HOME") != NULL)
        config = getenv("HOME") + std::string("/.config");
    else
        config = "./config";
    g_Config.memStickDirectory = config + "/ppsspp/";
    g_Config.flash0Directory   = File::GetExeDirectory() + "/flash0/";
    LogManager::Init();
    LogManager *logman = LogManager::GetInstance();
    g_Config.AddSearchPath(user_data_path);
    g_Config.AddSearchPath(g_Config.memStickDirectory + "PSP/SYSTEM/");
    g_Config.SetDefaultPath(g_Config.memStickDirectory + "PSP/SYSTEM/");
    g_Config.Load();
    g_Config.externalDirectory       = external_directory;
    const char          *fileToLog   = 0;
    const char          *stateToLoad = 0;
    bool                 gfxLog      = false;
    LogTypes::LOG_LEVELS logLevel    = LogTypes::LINFO;
    boot_filename                    = argv[1];
    skipLogo                         = true;
    std::unique_ptr<FileLoader> fileLoader(ConstructFileLoader(boot_filename));
    if (!fileLoader->Exists()) {
        fprintf(stderr, "File not found: %s\n", boot_filename.c_str());
        exit(1);
    }
    if (fileToLog != NULL)
        LogManager::GetInstance()->ChangeFileLog(fileToLog);
    if (g_Config.currentDirectory == "") {
        if (getenv("HOME") != NULL)
            g_Config.currentDirectory = getenv("HOME");
        else
            g_Config.currentDirectory = "./";
    }
    for (int i = 0; i < LogTypes::NUMBER_OF_LOGS; i++) {
        LogTypes::LOG_TYPE type = (LogTypes::LOG_TYPE)i;
        logman->SetEnable(type, true);
        logman->SetLogLevel(type, gfxLog && i == LogTypes::G3D ? LogTypes::LDEBUG : logLevel);
    }
    if (!gfxLog)
        logman->SetLogLevel(LogTypes::G3D, LogTypes::LERROR);
    const std::string langOverridePath = g_Config.memStickDirectory + "PSP/SYSTEM/lang/";
    if (!File::Exists(langOverridePath) || !File::IsDirectory(langOverridePath))
        i18nrepo.LoadIni(g_Config.sLanguageIni);
    else
        i18nrepo.LoadIni(g_Config.sLanguageIni, langOverridePath);
    I18NCategory *d = GetI18NCategory("DesktopUI");
    if (!boot_filename.empty() && stateToLoad != NULL)
        SaveState::Load(stateToLoad);
    screenManager = new ScreenManager();
    if (skipLogo) {
        screenManager->switchScreen(new EmuScreen(boot_filename));
    }
    std::string sysName = System_GetProperty(SYSPROP_NAME);
    isOuya              = KeyMap::IsOuya(sysName);
    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        gl_lost_manager_init();
    }
}
void NativeInitGraphics()
{
    FPU_SetFastMode();
    g_Config.iGPUBackend = GPU_BACKEND_OPENGL;

    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        thin3d = T3DCreateGLContext();
        CheckGLExtensions();
    }
    ui_theme.uiFont                          = UI::FontStyle(UBUNTU24, "", 20);
    ui_theme.uiFontSmall                     = UI::FontStyle(UBUNTU24, "", 14);
    ui_theme.uiFontSmaller                   = UI::FontStyle(UBUNTU24, "", 11);
    ui_theme.checkOn                         = I_CHECKEDBOX;
    ui_theme.checkOff                        = I_SQUARE;
    ui_theme.whiteImage                      = I_SOLIDWHITE;
    ui_theme.sliderKnob                      = I_CIRCLE;
    ui_theme.dropShadow4Grid                 = I_DROP_SHADOW;
    ui_theme.itemStyle.background            = UI::Drawable(0x55000000);
    ui_theme.itemStyle.fgColor               = 0xFFFFFFFF;
    ui_theme.itemFocusedStyle.background     = UI::Drawable(0xFFedc24c);
    ui_theme.itemDownStyle.background        = UI::Drawable(0xFFbd9939);
    ui_theme.itemDownStyle.fgColor           = 0xFFFFFFFF;
    ui_theme.itemDisabledStyle.background    = UI::Drawable(0x55E0D4AF);
    ui_theme.itemDisabledStyle.fgColor       = 0x80EEEEEE;
    ui_theme.itemHighlightedStyle.background = UI::Drawable(0x55bdBB39);
    ui_theme.itemHighlightedStyle.fgColor    = 0xFFFFFFFF;
    ui_theme.buttonStyle                     = ui_theme.itemStyle;
    ui_theme.buttonFocusedStyle              = ui_theme.itemFocusedStyle;
    ui_theme.buttonDownStyle                 = ui_theme.itemDownStyle;
    ui_theme.buttonDisabledStyle             = ui_theme.itemDisabledStyle;
    ui_theme.buttonHighlightedStyle          = ui_theme.itemHighlightedStyle;
    ui_theme.popupTitle.fgColor              = 0xFFE3BE59;
    ui_draw2d.Init(thin3d);
    ui_draw2d_front.Init(thin3d);

    uiTexture = thin3d->CreateTextureFromFile("ui_atlas.zim", T3DImageType::ZIM);
    if (!uiTexture) {
        PanicAlert("Failed to load ui_atlas.zim.\n\nPlace it in the directory \"assets\" under your PPSSPP directory.");
        ELOG("Failed to load ui_atlas.zim");
    }

    uiContext        = new UIContext();
    uiContext->theme = &ui_theme;
    uiContext->Init(thin3d, thin3d->GetShaderSetPreset(SS_TEXTURE_COLOR_2D), thin3d->GetShaderSetPreset(SS_COLOR_2D),
                    uiTexture, &ui_draw2d, &ui_draw2d_front);

    if (uiContext->Text())
        uiContext->Text()->SetFont("Tahoma", 20, 0);
    screenManager->setUIContext(uiContext);
    screenManager->setThin3DContext(thin3d);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}
void NativeShutdownGraphics()
{
    screenManager->deviceLost();
    uiTexture->Release();
    delete uiContext;
    uiContext = NULL;
    ui_draw2d.Shutdown();
    ui_draw2d_front.Shutdown();
    thin3d->Release();
}
void NativeRender()
{
    // g_GameManager.Update();
    thin3d->Clear(T3DClear::COLOR | T3DClear::DEPTH | T3DClear::STENCIL, 0xFF000000, 0.0f, 0);
    T3DViewport viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = pixel_xres;
    viewport.Height   = pixel_yres;
    viewport.MaxDepth = 1.0;
    viewport.MinDepth = 0.0;
    thin3d->SetViewports(1, &viewport);
    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        glstate.depthWrite.set(GL_TRUE);
        glstate.colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glstate.Restore();
    }
    thin3d->SetTargetSize(pixel_xres, pixel_yres);
    float     xres = dp_xres;
    float     yres = dp_yres;
    Matrix4x4 ortho;
    if (g_Config.iGPUBackend == GPU_BACKEND_DIRECT3D9) {
        ortho.setOrthoD3D(0.0f, xres, yres, 0.0f, -1.0f, 1.0f);
        Matrix4x4 translation;
        translation.setTranslation(Vec3(-0.5f, -0.5f, 0.0f));
        ortho = translation * ortho;
    } else {
        ortho.setOrtho(0.0f, xres, yres, 0.0f, -1.0f, 1.0f);
    }
    ui_draw2d.SetDrawMatrix(ortho);
    ui_draw2d_front.SetDrawMatrix(ortho);
    screenManager->render();
    if (screenManager->getUIContext()->Text()) {
        screenManager->getUIContext()->Text()->OncePerFrame();
    }

    thin3d->SetScissorEnabled(false);
    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        glstate.depthWrite.set(GL_TRUE);
        glstate.colorMask.set(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    if (resized) {
        resized = false;
    }
}
void NativeUpdate(InputState &input)
{
    {
        lock_guard lock(pendingMutex);
        for (size_t i = 0; i < pendingMessages.size(); i++) {
            if (pendingMessages[i].msg == "inputDeviceConnected") {
                KeyMap::NotifyPadConnected(pendingMessages[i].value);
            }
            screenManager->sendMessage(pendingMessages[i].msg.c_str(), pendingMessages[i].value.c_str());
        }
        pendingMessages.clear();
    }
    screenManager->update(input);
}
bool NativeKey(const KeyInput &key)
{
    if (g_Config.bPauseExitsEmulator) {
        static std::vector<int> pspKeys;
        pspKeys.clear();
        if (KeyMap::KeyToPspButton(key.deviceId, key.keyCode, &pspKeys)) {
            if (std::find(pspKeys.begin(), pspKeys.end(), VIRTKEY_PAUSE) != pspKeys.end()) {
                System_SendMessage("finish", "");
                return true;
            }
        }
    }

    g_buttonTracker.Process(key);
    bool retval = false;
    if (screenManager)
        retval = screenManager->key(key);
    return retval;
}
void NativeResized()
{
    resized = true;
    if (uiContext) {
        uiContext->SetBounds(Bounds(0, 0, dp_xres, dp_yres));
    }
}
void NativeShutdown()
{
    if (g_Config.iGPUBackend == GPU_BACKEND_OPENGL) {
        gl_lost_manager_shutdown();
    }
    screenManager->shutdown();
    delete screenManager;
    screenManager = 0;
    g_Config.Save();
    LogManager::Shutdown();
    ILOG("NativeShutdown called");
    System_SendMessage("finish", "");
}





void System_SendMessage(const char *command, const char *parameter)
{
    if (!strcmp(command, "toggle_fullscreen")) {
        g_ToggleFullScreenNextFrame = true;
    } else if (!strcmp(command, "finish")) {
        g_QuitRequested = true;
    }
}
std::string System_GetProperty(SystemProperty prop)
{
    switch (prop) {
        case SYSPROP_NAME:
            return "SDL:Linux";
        case SYSPROP_LANGREGION:
            return "en_US";
        default:
            return "";
    }
}
int System_GetPropertyInt(SystemProperty prop)
{
    switch (prop) {
        case SYSPROP_AUDIO_SAMPLE_RATE:
            return 44100;
        case SYSPROP_DISPLAY_REFRESH_RATE:
            return 60000;
        default:
            return -1;
    }
}
extern void mixaudio(void *userdata, Uint8 *stream, int len)
{
    NativeMix((short *)stream, len / 4);
}




float dpi_scale = 1.0f;

void main_loop()
{
    int       framecount  = 0;
    float     t           = 0;
    float     lastT       = 0;
    uint32_t  pad_buttons = 0;
    SDL_Event event;

    while (!g_QuitRequested) {
        input_state.accelerometer_valid = false;
        input_state.mouse_valid         = true;

        while (SDL_PollEvent(&event)) {
            float mx = event.motion.x * g_dpi_scale;
            float my = event.motion.y * g_dpi_scale;

            switch (event.type) {
                case SDL_QUIT:
                    g_QuitRequested = 1;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED: {
                            Uint32 window_flags = SDL_GetWindowFlags(g_Screen);
                            bool   fullscreen   = (window_flags & SDL_WINDOW_FULLSCREEN);
                            pixel_xres          = event.window.data1;
                            pixel_yres          = event.window.data2;
                            dp_xres             = (float)pixel_xres * dpi_scale;
                            dp_yres             = (float)pixel_yres * dpi_scale;
                            NativeResized();

                            if (lastUIState == UISTATE_INGAME && fullscreen && !g_Config.bShowTouchControls) {
                                SDL_ShowCursor(SDL_DISABLE);
                            } else if (lastUIState != UISTATE_INGAME || !fullscreen) {
                                SDL_ShowCursor(SDL_ENABLE);
                            }
                            break;
                        } break;
                    }
                case SDL_KEYDOWN: {
                    int      k = event.key.keysym.sym;
                    KeyInput key;
                    key.flags    = KEY_DOWN;
                    key.keyCode  = KeyMapRawSDLtoNative.find(k)->second;
                    key.deviceId = DEVICE_ID_KEYBOARD;
                    NativeKey(key);
                    for (int i = 0; i < ARRAY_SIZE(legacyKeyMap); i++) {
                        if (legacyKeyMap[i] == key.keyCode)
                            pad_buttons |= 1 << i;
                    }
                    break;
                }
                case SDL_KEYUP: {
                    int      k = event.key.keysym.sym;
                    KeyInput key;
                    key.flags    = KEY_UP;
                    key.keyCode  = KeyMapRawSDLtoNative.find(k)->second;
                    key.deviceId = DEVICE_ID_KEYBOARD;
                    NativeKey(key);
                    for (int i = 0; i < ARRAY_SIZE(legacyKeyMap); i++) {
                        if (legacyKeyMap[i] == key.keyCode)
                            pad_buttons &= ~(1 << i);
                    }
                    break;
                }
                case SDL_TEXTINPUT: {
                    int      pos = 0;
                    int      c   = u8_nextchar(event.text.text, &pos);
                    KeyInput key;
                    key.flags    = KEY_CHAR;
                    key.keyCode  = c;
                    key.deviceId = DEVICE_ID_KEYBOARD;
                    NativeKey(key);
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT: {
                            input_state.pointer_x[0]    = mx;
                            input_state.pointer_y[0]    = my;
                            input_state.pointer_down[0] = true;
                            input_state.mouse_valid     = true;
                            TouchInput input;
                            input.x     = mx;
                            input.y     = my;
                            input.flags = TOUCH_DOWN | TOUCH_MOUSE;
                            input.id    = 0;

                            KeyInput key(DEVICE_ID_MOUSE, NKCODE_EXT_MOUSEBUTTON_1, KEY_DOWN);
                            NativeKey(key);
                        } break;
                        case SDL_BUTTON_RIGHT: {
                            KeyInput key(DEVICE_ID_MOUSE, NKCODE_EXT_MOUSEBUTTON_2, KEY_DOWN);
                            NativeKey(key);
                        } break;
                    }
                    break;
                case SDL_MOUSEWHEEL: {
                    KeyInput key;
                    key.deviceId = DEVICE_ID_MOUSE;
                    if (event.wheel.y > 0) {
                        key.keyCode = NKCODE_EXT_MOUSEWHEEL_UP;
                    } else {
                        key.keyCode = NKCODE_EXT_MOUSEWHEEL_DOWN;
                    }
                    key.flags = KEY_DOWN;
                    NativeKey(key);
                    key.flags = KEY_UP;
                    NativeKey(key);
                }
                case SDL_MOUSEMOTION:
                    if (input_state.pointer_down[0]) {
                        input_state.pointer_x[0] = mx;
                        input_state.pointer_y[0] = my;
                        input_state.mouse_valid  = true;
                        TouchInput input;
                        input.x     = mx;
                        input.y     = my;
                        input.flags = TOUCH_MOVE | TOUCH_MOUSE;
                        input.id    = 0;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT: {
                            input_state.pointer_x[0]    = mx;
                            input_state.pointer_y[0]    = my;
                            input_state.pointer_down[0] = false;
                            input_state.mouse_valid     = true;
                            TouchInput input;
                            input.x     = mx;
                            input.y     = my;
                            input.flags = TOUCH_UP | TOUCH_MOUSE;
                            input.id    = 0;

                            KeyInput key(DEVICE_ID_MOUSE, NKCODE_EXT_MOUSEBUTTON_1, KEY_UP);
                            NativeKey(key);
                        } break;
                        case SDL_BUTTON_RIGHT: {
                            KeyInput key(DEVICE_ID_MOUSE, NKCODE_EXT_MOUSEBUTTON_2, KEY_UP);
                            NativeKey(key);
                        } break;
                    }
                    break;
                default:
                    // joystick->ProcessInput(event);
                    break;
            }
        }

        const uint8 *keys       = SDL_GetKeyboardState(NULL);
        input_state.pad_buttons = pad_buttons;
        UpdateInputState(&input_state, true);
        UpdateRunLoop();

        if (g_QuitRequested)
            break;
        if (lastUIState != GetUIState()) {
            lastUIState = GetUIState();
            if (lastUIState == UISTATE_INGAME && g_Config.bFullScreen && !g_Config.bShowTouchControls)
                SDL_ShowCursor(SDL_DISABLE);
            if (lastUIState != UISTATE_INGAME && g_Config.bFullScreen)
                SDL_ShowCursor(SDL_ENABLE);
        }

        if (!keys[SDLK_TAB] || t - lastT >= 1.0 / 60.0) {
            SDL_GL_SwapWindow(g_Screen);
            lastT = t;
        }

        time_update();
        t = time_now();
        framecount++;
    }
}
int main(int argc, char *argv[])
{
    putenv((char *)"SDL_VIDEO_CENTERED=1");
    std::string app_name      = "psp";
    std::string app_name_nice = "PSP";
    std::string version       = PPSSPP_GIT_VERSION;
    bool        landscape     = true;

    SDL_DisplayMode displayMode;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
    int should_be_zero = SDL_GetCurrentDisplayMode(0, &displayMode);

    g_DesktopWidth  = displayMode.w;
    g_DesktopHeight = displayMode.h;
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    Uint32 mode = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    bool  portrait  = false;
    bool  set_ipad  = false;
    float set_dpi   = 1.0f;
    float set_scale = 1.0f;

    pixel_xres = 480 * 2 * set_scale;
    pixel_yres = 272 * 2 * set_scale;

    if (portrait) {
        std::swap(pixel_xres, pixel_yres);
    }

    set_dpi = 1.0f / set_dpi;
    if (set_ipad) {
        pixel_xres = 1024;
        pixel_yres = 768;
    }

    if (!landscape) {
        std::swap(pixel_xres, pixel_yres);
    }


    dp_xres  = (float)pixel_xres * dpi_scale;
    dp_yres  = (float)pixel_yres * dpi_scale;
    g_Screen = SDL_CreateWindow(app_name_nice.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pixel_xres,
                                pixel_yres, mode);

    SDL_GLContext glContext = SDL_GL_CreateContext(g_Screen);

    glewInit();
    char path[2048];

    const char *the_path = getenv("HOME");
    if (!the_path) {
        struct passwd *pwd = getpwuid(getuid());
        if (pwd)
            the_path = pwd->pw_dir;
    }

    strcpy(path, the_path);
    if (path[strlen(path) - 1] != '/')
        strcat(path, "/");

    NativeInit(argc, (const char **)argv, path, "/tmp", "BADCOFFEE");
    pixel_in_dps = (float)pixel_xres / dp_xres;
    g_dpi_scale  = dp_xres / (float)pixel_xres;
    NativeInitGraphics();
    NativeResized();
    SDL_AudioSpec fmt, ret_fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.freq     = 44100;
    fmt.format   = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples  = 2048;
    fmt.callback = &mixaudio;
    fmt.userdata = (void *)0;
    SDL_OpenAudio(&fmt, &ret_fmt);
    SDL_PauseAudio(0);

    EnableFZ();

    main_loop();

    NativeShutdownGraphics();
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    NativeShutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_Quit();
    return 0;
}
