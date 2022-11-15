#pragma once
#include <string>
// #include "util/const_map.h"
#include <map>
#include <SDL2/SDL.h>
#include "input/keycodes.h"

enum
{
    HAPTIC_SOFT_KEYBOARD        = -1,
    HAPTIC_VIRTUAL_KEY          = -2,
    HAPTIC_LONG_PRESS_ACTIVATED = -3,
};
enum SystemProperty
{
    SYSPROP_NAME,
    SYSPROP_LANGREGION,
    SYSPROP_CPUINFO,
    SYSPROP_CLIPBOARD_TEXT,
    SYSPROP_GPUDRIVER_VERSION,
    SYSPROP_SYSTEMVERSION,
    SYSPROP_DISPLAY_XRES,
    SYSPROP_DISPLAY_YRES,
    SYSPROP_DISPLAY_REFRESH_RATE,
    SYSPROP_MOGA_VERSION,
    SYSPROP_AUDIO_SAMPLE_RATE,
    SYSPROP_AUDIO_FRAMES_PER_BUFFER,
    SYSPROP_AUDIO_OPTIMAL_SAMPLE_RATE,
    SYSPROP_AUDIO_OPTIMAL_FRAMES_PER_BUFFER,
};

extern const char *PPSSPP_GIT_VERSION;




int init(int argc, char *argv[]);

struct InputState;
struct TouchInput;
struct KeyInput;
struct AxisInput;

void NativeInit(int argc, const char *argv[], const char *savegame_directory, const char *external_directory,
                const char *installID, bool fs = false);
void NativeInitGraphics();
void NativeResized();
void NativeUpdate(InputState &input);

bool NativeKey(const KeyInput &key);

void NativeRender();
int  NativeMix(short *audio, int num_samples);
void NativeSetMixer(void *mixer);
void NativeShutdownGraphics();
void NativeShutdown();
void NativeRestoreState(bool firstTime);
void NativeSaveState();

void ShowAd(int x, int y, bool center_x);

bool System_InputBoxGetString(const char *title, const char *defaultValue, char *outValue, size_t outlength);
bool System_InputBoxGetWString(const wchar_t *title, const std::wstring &defaultValue, std::wstring &outValue);
void System_SendMessage(const char *command, const char *parameter);

std::string System_GetProperty(SystemProperty prop);
int         System_GetPropertyInt(SystemProperty prop);





template <typename T, typename U> class InitConstMap {
  private:
    std::map<T, U> m_map;

  public:
    InitConstMap(const T &key, const U &val)
    {
        m_map[key] = val;
    }

    InitConstMap<T, U> &operator()(const T &key, const U &val)
    {
        m_map[key] = val;
        return *this;
    }

    operator std::map<T, U>()
    {
        return m_map;
    }
};



// TODO: Add any missing keys
static const std::map<int, int> KeyMapRawSDLtoNative = InitConstMap<int, int>(SDLK_p, NKCODE_P)(SDLK_o, NKCODE_O)(
    SDLK_i, NKCODE_I)(SDLK_u, NKCODE_U)(SDLK_y, NKCODE_Y)(SDLK_t, NKCODE_T)(SDLK_r, NKCODE_R)(SDLK_e, NKCODE_E)(
    SDLK_w, NKCODE_W)(SDLK_q, NKCODE_Q)(SDLK_l, NKCODE_L)(SDLK_k, NKCODE_K)(SDLK_j, NKCODE_J)(SDLK_h, NKCODE_H)(
    SDLK_g, NKCODE_G)(SDLK_f, NKCODE_F)(SDLK_d, NKCODE_D)(SDLK_s, NKCODE_S)(SDLK_a, NKCODE_A)(SDLK_m, NKCODE_M)(
    SDLK_n, NKCODE_N)(SDLK_b, NKCODE_B)(SDLK_v, NKCODE_V)(SDLK_c, NKCODE_C)(SDLK_x, NKCODE_X)(SDLK_z, NKCODE_Z)(
    SDLK_COMMA, NKCODE_COMMA)(SDLK_PERIOD, NKCODE_PERIOD)(SDLK_LALT, NKCODE_ALT_LEFT)(SDLK_RALT, NKCODE_ALT_RIGHT)(
    SDLK_LSHIFT, NKCODE_SHIFT_LEFT)(SDLK_RSHIFT, NKCODE_SHIFT_RIGHT)(SDLK_TAB, NKCODE_TAB)(SDLK_SPACE, NKCODE_SPACE)(
    SDLK_RETURN, NKCODE_ENTER)(SDLK_MINUS, NKCODE_MINUS)(SDLK_EQUALS, NKCODE_EQUALS)(
    SDLK_LEFTBRACKET, NKCODE_LEFT_BRACKET)(SDLK_RIGHTBRACKET, NKCODE_RIGHT_BRACKET)(SDLK_BACKSLASH, NKCODE_BACKSLASH)(
    SDLK_SEMICOLON, NKCODE_SEMICOLON)(SDLK_SLASH, NKCODE_SLASH)(SDLK_AT, NKCODE_AT)(SDLK_PLUS, NKCODE_PLUS)(
    SDLK_PAGEUP, NKCODE_PAGE_UP)(SDLK_PAGEDOWN, NKCODE_PAGE_DOWN)(SDLK_ESCAPE, NKCODE_ESCAPE)(
    SDLK_BACKSPACE, NKCODE_DEL)(SDLK_DELETE, NKCODE_FORWARD_DEL)(SDLK_LCTRL, NKCODE_CTRL_LEFT)(
    SDLK_RCTRL, NKCODE_CTRL_RIGHT)(SDLK_CAPSLOCK, NKCODE_CAPS_LOCK)(SDLK_HOME, NKCODE_MOVE_HOME)(
    SDLK_END, NKCODE_MOVE_END)(SDLK_INSERT, NKCODE_INSERT)(SDLK_KP_0, NKCODE_NUMPAD_0)(SDLK_KP_1, NKCODE_NUMPAD_1)(
    SDLK_KP_2, NKCODE_NUMPAD_2)(SDLK_KP_3, NKCODE_NUMPAD_3)(SDLK_KP_4, NKCODE_NUMPAD_4)(SDLK_KP_5, NKCODE_NUMPAD_5)(
    SDLK_KP_6, NKCODE_NUMPAD_6)(SDLK_KP_7, NKCODE_NUMPAD_7)(SDLK_KP_8, NKCODE_NUMPAD_8)(SDLK_KP_9, NKCODE_NUMPAD_9)(
    SDLK_KP_DIVIDE, NKCODE_NUMPAD_DIVIDE)(SDLK_KP_MULTIPLY, NKCODE_NUMPAD_MULTIPLY)(
    SDLK_KP_MINUS, NKCODE_NUMPAD_SUBTRACT)(SDLK_KP_PLUS, NKCODE_NUMPAD_ADD)(SDLK_KP_PERIOD, NKCODE_NUMPAD_DOT)(
    SDLK_KP_ENTER, NKCODE_NUMPAD_ENTER)(SDLK_KP_EQUALS, NKCODE_NUMPAD_EQUALS)(SDLK_1, NKCODE_1)(SDLK_2, NKCODE_2)(
    SDLK_3, NKCODE_3)(SDLK_4, NKCODE_4)(SDLK_5, NKCODE_5)(SDLK_6, NKCODE_6)(SDLK_7, NKCODE_7)(SDLK_8, NKCODE_8)(
    SDLK_9, NKCODE_9)(SDLK_0, NKCODE_0)(SDLK_F1, NKCODE_F1)(SDLK_F2, NKCODE_F2)(SDLK_F3, NKCODE_F3)(SDLK_F4, NKCODE_F4)(
    SDLK_F5, NKCODE_F5)(SDLK_F6, NKCODE_F6)(SDLK_F7, NKCODE_F7)(SDLK_F8, NKCODE_F8)(SDLK_F9, NKCODE_F9)(
    SDLK_F10, NKCODE_F10)(SDLK_F11, NKCODE_F11)(SDLK_F12, NKCODE_F12)(SDLK_LEFT, NKCODE_DPAD_LEFT)(
    SDLK_UP, NKCODE_DPAD_UP)(SDLK_RIGHT, NKCODE_DPAD_RIGHT)(SDLK_DOWN, NKCODE_DPAD_DOWN);
