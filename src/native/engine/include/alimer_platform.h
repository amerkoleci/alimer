// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_PLATFORM_H_
#define ALIMER_PLATFORM_H_ 1

#include "alimer_types.h"

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#   pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4201) // nameless struct/union
#endif

/* Forward */
typedef struct Window Window;

/* Enums + Flags */
typedef enum ButtonState {
    ButtonState_None,
    ButtonState_Pressed,
    ButtonState_Released,

    _ButtonState_Force32 = 0x7FFFFFFF
} ButtonState;

typedef enum MouseButton {
    MouseButton_None,
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Middle,
    MouseButton_X1,
    MouseButton_X2,

    MouseButton_Count,
    _MouseButton_Force32 = 0x7FFFFFFF
} MouseButton;

typedef enum KeyboardKey {
    KeyboardKey_None = 0,

    KeyboardKey_Backspace = 0x08,
    KeyboardKey_Tab = 0x09,
    KeyboardKey_Clear = 0x0C,
    /// Return/ENTER key
    KeyboardKey_Return = 0x0D,

    KeyboardKey_Pause = 0x13,
    KeyboardKey_CapsLock = 0x14,
    KeyboardKey_Kana = 0x15,
    KeyboardKey_ImeOn = 0x16,

    KeyboardKey_Kanji = 0x19,

    KeyboardKey_ImeOff = 0x1a,
    KeyboardKey_Escape = 0x1b,
    KeyboardKey_ImeConvert = 0x1c,
    KeyboardKey_ImeNoConvert = 0x1d,

    KeyboardKey_Space = 0x20,
    KeyboardKey_PageUp = 0x21,
    KeyboardKey_PageDown = 0x22,
    KeyboardKey_End = 0x23,
    KeyboardKey_Home = 0x24,
    KeyboardKey_Left = 0x25,
    KeyboardKey_Up = 0x26,
    KeyboardKey_Right = 0x27,
    KeyboardKey_Down = 0x28,
    KeyboardKey_Select = 0x29,
    KeyboardKey_Print = 0x2a,
    KeyboardKey_Execute = 0x2b,
    KeyboardKey_PrintScreen = 0x2c,
    KeyboardKey_Insert = 0x2d,
    KeyboardKey_Delete = 0x2e,
    KeyboardKey_Help = 0x2f,
    KeyboardKey_D0 = 0x30,
    KeyboardKey_D1 = 0x31,
    KeyboardKey_D2 = 0x32,
    KeyboardKey_D3 = 0x33,
    KeyboardKey_D4 = 0x34,
    KeyboardKey_D5 = 0x35,
    KeyboardKey_D6 = 0x36,
    KeyboardKey_D7 = 0x37,
    KeyboardKey_D8 = 0x38,
    KeyboardKey_D9 = 0x39,

    KeyboardKey_A = 0x41,
    KeyboardKey_B = 0x42,
    KeyboardKey_C = 0x43,
    KeyboardKey_D = 0x44,
    KeyboardKey_E = 0x45,
    KeyboardKey_F = 0x46,
    KeyboardKey_G = 0x47,
    KeyboardKey_H = 0x48,
    KeyboardKey_I = 0x49,
    KeyboardKey_J = 0x4a,
    KeyboardKey_K = 0x4b,
    KeyboardKey_L = 0x4c,
    KeyboardKey_M = 0x4d,
    KeyboardKey_N = 0x4e,
    KeyboardKey_O = 0x4f,
    KeyboardKey_P = 0x50,
    KeyboardKey_Q = 0x51,
    KeyboardKey_R = 0x52,
    KeyboardKey_S = 0x53,
    KeyboardKey_T = 0x54,
    KeyboardKey_U = 0x55,
    KeyboardKey_V = 0x56,
    KeyboardKey_W = 0x57,
    KeyboardKey_X = 0x58,
    KeyboardKey_Y = 0x59,
    KeyboardKey_Z = 0x5a,

    KeyboardKey_LeftSuper = 0x5b, /* Left Windows */
    KeyboardKey_RightSuper = 0x5c, /* Right Windows */
    KeyboardKey_Apps = 0x5d,
    KeyboardKey_Sleep = 0x5f,

    KeyboardKey_Numpad0 = 0x60,
    KeyboardKey_Numpad1 = 0x61,
    KeyboardKey_Numpad2 = 0x62,
    KeyboardKey_Numpad3 = 0x63,
    KeyboardKey_Numpad4 = 0x64,
    KeyboardKey_Numpad5 = 0x65,
    KeyboardKey_Numpad6 = 0x66,
    KeyboardKey_Numpad7 = 0x67,
    KeyboardKey_Numpad8 = 0x68,
    KeyboardKey_Numpad9 = 0x69,
    KeyboardKey_Multiply = 0x6A,
    KeyboardKey_Add = 0x6B,
    KeyboardKey_Separator = 0x6C,
    KeyboardKey_Subtract = 0x6D,
    KeyboardKey_Decimal = 0x6E,
    KeyboardKey_Divide = 0x6F,

    KeyboardKey_F1 = 0x70,
    KeyboardKey_F2 = 0x71,
    KeyboardKey_F3 = 0x72,
    KeyboardKey_F4 = 0x73,
    KeyboardKey_F5 = 0x74,
    KeyboardKey_F6 = 0x75,
    KeyboardKey_F7 = 0x76,
    KeyboardKey_F8 = 0x77,
    KeyboardKey_F9 = 0x78,
    KeyboardKey_F10 = 0x79,
    KeyboardKey_F11 = 0x7A,
    KeyboardKey_F12 = 0x7B,
    KeyboardKey_F13 = 0x7C,
    KeyboardKey_F14 = 0x7D,
    KeyboardKey_F15 = 0x7E,
    KeyboardKey_F16 = 0x7F,
    KeyboardKey_F17 = 0x80,
    KeyboardKey_F18 = 0x81,
    KeyboardKey_F19 = 0x82,
    KeyboardKey_F20 = 0x83,
    KeyboardKey_F21 = 0x84,
    KeyboardKey_F22 = 0x85,
    KeyboardKey_F23 = 0x86,
    KeyboardKey_F24 = 0x87,

    KeyboardKey_NumLock = 0x90,
    KeyboardKey_ScrollLock = 0x91,

    KeyboardKey_LeftShift = 0xa0,
    KeyboardKey_RightShift = 0xa1,
    KeyboardKey_LeftControl = 0xa2,
    KeyboardKey_RightControl = 0xa3,
    KeyboardKey_LeftAlt = 0xa4,
    KeyboardKey_RightAlt = 0xa5,
    KeyboardKey_BrowserBack = 0xa6,
    KeyboardKey_BrowserForward = 0xa7,
    KeyboardKey_BrowserRefresh = 0xa8,
    KeyboardKey_BrowserStop = 0xa9,
    KeyboardKey_BrowserSearch = 0xaa,
    KeyboardKey_BrowserFavorites = 0xab,
    KeyboardKey_BrowserHome = 0xac,
    KeyboardKey_VolumeMute = 0xad,
    KeyboardKey_VolumeDown = 0xae,
    KeyboardKey_VolumeUp = 0xaf,
    KeyboardKey_MediaNextTrack = 0xb0,
    KeyboardKey_MediaPreviousTrack = 0xb1,
    KeyboardKey_MediaStop = 0xb2,
    KeyboardKey_MediaPlayPause = 0xb3,
    KeyboardKey_LaunchMail = 0xb4,
    KeyboardKey_SelectMedia = 0xb5,
    KeyboardKey_LaunchApplication1 = 0xb6,
    KeyboardKey_LaunchApplication2 = 0xb7,

    KeyboardKey_OemSemicolon = 0xba,
    KeyboardKey_OemPlus = 0xbb,
    KeyboardKey_OemComma = 0xbc,
    KeyboardKey_OemMinus = 0xbd,
    KeyboardKey_OemPeriod = 0xbe,
    KeyboardKey_OemQuestion = 0xbf,
    KeyboardKey_OemTilde = 0xc0,
    KeyboardKey_OemOpenBrackets = 0xdb,
    KeyboardKey_OemPipe = 0xdc,
    KeyboardKey_OemCloseBrackets = 0xdd,
    KeyboardKey_OemQuotes = 0xde,
    KeyboardKey_Oem8 = 0xdf,
    KeyboardKey_OemBackslash = 0xe2,

    KeyboardKey_ProcessKey = 0xe5,

    KeyboardKey_OemCopy = 0xf2,
    KeyboardKey_OemAuto = 0xf3,
    KeyboardKey_OemEnlW = 0xf4,

    KeyboardKey_Attn = 0xf6,
    KeyboardKey_Crsel = 0xf7,
    KeyboardKey_Exsel = 0xf8,
    KeyboardKey_EraseEof = 0xf9,
    KeyboardKey_Play = 0xfa,
    KeyboardKey_Zoom = 0xfb,

    KeyboardKey_Pa1 = 0xfd,
    KeyboardKey_OemClear = 0xfe,

    _KeyboardKey_Count,
    _KeyboardKey_Force32 = 0x7FFFFFFF
} KeyboardKey;

typedef enum EventType {
    EventType_Unknown = 0,
    EventType_Quit,
    EventType_Terminating,
    EventType_LowMemory,
    EventType_WillEnterBackground,
    EventType_DidEnterBackground,
    EventType_WillEnterForeground,
    EventType_DidEnterForeground,
    EventType_LocaleChanged,
    EventType_SystemThemeChanged,

    EventType_Window,
    EventType_KeyDown,
    EventType_KeyUp,
    EventType_TextInput,

    EventType_MouseMotion,
    EventType_MouseButtonDown,
    EventType_MouseButtonUp,
    EventType_MouseWheel,
    EventType_MouseAdded,
    EventType_MouseRemoved,

    EventType_ClipboardUpdate,

    _EventType_Count,
    _EventType_Force32 = 0x7FFFFFFF
} EventType;

typedef enum WindowEventType {
    WindowEventType_None = 0,
    WindowEventType_Shown,
    WindowEventType_Hidden,
    WindowEventType_Exposed,
    WindowEventType_Moved,
    WindowEventType_Resized,
    WindowEventType_SizeChanged,
    WindowEventType_Minimized,
    WindowEventType_Maximized,
    WindowEventType_Restored,
    WindowEventType_MouseEnter,
    WindowEventType_MouseLeave,
    WindowEventType_FocusGained,
    WindowEventType_FocusLost,
    WindowEventType_CloseRequested,
    WindowEventType_DisplayChanged,
    WindowEventType_DisplayScaleChanged,
    WindowEventType_SafeAreaChanged,
    WindowEventType_Occluded,
    WindowEventType_EnterFullscreen,
    WindowEventType_LeaveFullscreen,

    _WindowEventType_Count,
    _WindowEventType_Force32 = 0x7FFFFFFF
} WindowEventType;

typedef enum PowerLineStatus {
    PowerLineStatus_Unknown,
    PowerLineStatus_Online,
    PowerLineStatus_Offline,

    _PowerLineStatus_Force32 = 0x7FFFFFFF
} PowerLineStatus;

typedef enum BatteryStatus {
    BatteryStatus_Unknown,
    BatteryStatus_NotPresent,
    BatteryStatus_OnBattery,
    BatteryStatus_Charging,
    BatteryStatus_Charged,

    _BatteryStatus_Force32 = 0x7FFFFFFF
} BatteryStatus;

typedef Flags WindowFlags;
static const WindowFlags WindowFlags_None = 0x0000;
static const WindowFlags WindowFlags_Fullscreen = 0x0001;
static const WindowFlags WindowFlags_Hidden = 0x0002;
static const WindowFlags WindowFlags_Borderless = 0x0004;
static const WindowFlags WindowFlags_Resizable = 0x0008;
static const WindowFlags WindowFlags_Maximized = 0x0010;
static const WindowFlags WindowFlags_AlwaysOnTop = 0x0020;

typedef Flags KeyModifiers;
static const KeyModifiers KeyModifiers_None = 0x0000;
static const KeyModifiers KeyModifiers_Control = 0x0001;
static const KeyModifiers KeyModifiers_Shift = 0x0002;
static const KeyModifiers KeyModifiers_Alt = 0x0004;
static const KeyModifiers KeyModifiers_Super = 0x0008;
static const KeyModifiers KeyModifiers_CapsLock = 0x0010;
static const KeyModifiers KeyModifiers_NumLock = 0x0020;

/* Structs */
typedef struct WindowIcon {
    uint32_t    width;
    uint32_t    height;
    uint8_t* data;
} WindowIcon;

typedef struct WindowDesc {
    const char* title;
    uint32_t    width;
    uint32_t    height;
    WindowFlags flags;
    WindowIcon  icon;
} WindowDesc;

typedef struct WindowEvent {
    WindowEventType type;
    uint32_t windowID;
    int32_t data1;
    int32_t data2;
} WindowEvent;

typedef struct KeyEvent {
    uint32_t windowID;
    KeyboardKey key;
    KeyModifiers modifiers;
} KeyEvent;

typedef struct TextInputEvent {
    uint32_t windowID;
    const char* text;
} TextInputEvent;

typedef struct MouseMotionEvent {
    uint32_t windowID;
    float x;
    float y;
    float xRelative;
    float yRelative;
} MouseMotionEvent;

typedef struct MouseButtonEvent {
    uint32_t windowID;
    float x;
    float y;
    MouseButton button;
} MouseButtonEvent;

typedef struct MouseWheelEvent {
    uint32_t windowID;
    float x;
    float y;
} MouseWheelEvent;

typedef struct PlatformEvent {
    EventType       type;
    union
    {
        WindowEvent window;
        KeyEvent key;
        TextInputEvent text;
        MouseMotionEvent motion;
        MouseButtonEvent button;
        MouseWheelEvent wheel;
    };
} PlatformEvent;

/* Platform */
ALIMER_API bool alimerPlatformInit(void);
ALIMER_API void alimerPlatformShutdown(void);
ALIMER_API bool alimerPlatformPollEvent(PlatformEvent* evt);
ALIMER_API void alimerPlatformGetMousePosition(float* x, float* y);

/* Window */
ALIMER_API Window* alimerWindowCreate(const WindowDesc* desc);
ALIMER_API void alimerWindowDestroy(Window* window);
ALIMER_API uint32_t alimerWindowGetID(Window* window);
ALIMER_API bool alimerWindowIsOpen(Window* window);
ALIMER_API void alimerWindowSetPosition(Window* window, int32_t x, int32_t y);
ALIMER_API void alimerWindowGetPosition(Window* window, int32_t* x, int32_t* y);
ALIMER_API void alimerWindowSetCentered(Window* window);
ALIMER_API void alimerWindowSetSize(Window* window, uint32_t width, uint32_t height);
ALIMER_API void alimerWindowGetSize(Window* window, uint32_t* width, uint32_t* height);
ALIMER_API void alimerWindowGetSizeInPixels(Window* window, uint32_t* width, uint32_t* height);
ALIMER_API void alimerWindowSetTitle(Window* window, const char* title);
ALIMER_API const char* alimerWindowGetTitle(Window* window);
ALIMER_API float alimerWindowGetDisplayScale(Window* window);
ALIMER_API bool alimerWindowGetMousePosition(Window* window, float* x, float* y);

ALIMER_API bool alimerWindowIsMinimized(Window* window);
ALIMER_API bool alimerWindowIsMaximized(Window* window);
ALIMER_API bool alimerWindowIsFullscreen(Window* window);
ALIMER_API void alimerWindowSetFullscreen(Window* window, bool value);
ALIMER_API bool alimerWindowHasFocus(Window* window);
ALIMER_API void alimerWindowShow(Window* window);
ALIMER_API void alimerWindowHide(Window* window);
ALIMER_API void alimerWindowMaximize(Window* window);
ALIMER_API void alimerWindowMinimize(Window* window);
ALIMER_API void alimerWindowRestore(Window* window);
ALIMER_API void alimerWindowFocus(Window* window);

ALIMER_API void* alimerWindowGetNativeDisplay(Window* window);
ALIMER_API void* alimerWindowGetNativeHandle(Window* window);

/* Clipboard */
ALIMER_API bool alimerHasClipboardText(void);
ALIMER_API const char* alimerClipboardGetText(void);
ALIMER_API void alimerClipboardSetText(const char* text);

/* PowerStatus */
ALIMER_API PowerLineStatus alimerGetPowerLineStatus(void);
ALIMER_API BatteryStatus alimerGetBatteryStatus(int* batteryLifeTime, int* batteryLifePercent);

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif

#endif /* ALIMER_PLATFORM_H_ */
