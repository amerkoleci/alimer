// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_H_
#define ALIMER_H_ 1

#include "alimer_platform.h"

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#   pragma clang diagnostic ignored "-Wnested-anon-types"
#elif defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4201) // nameless struct/union
#endif

/* Version API */
#define ALIMER_VERSION_MAJOR    1
#define ALIMER_VERSION_MINOR    0
#define ALIMER_VERSION_PATCH	0

#define MAX_LOG_MESSAGE_SIZE        1024

#ifdef __cplusplus
#   define DEFAULT_INITIALIZER(x) = x
#else
#   define DEFAULT_INITIALIZER(x)
#endif

/* Forward declarations */
typedef struct Window Window;
typedef struct Image Image;
typedef struct Font Font;

/* Enums */
typedef enum LogCategory {
    LogCategory_System = 0,
    LogCategory_Platform,
    LogCategory_GPU,
    LogCategory_Audio,
    LogCategory_Physics,

    LogCategory_Count,
    _LogCategory_Force32 = 0x7FFFFFFF
} LogCategory;

typedef enum LogLevel {
    LogLevel_Off = 0,
    LogLevel_Trace = 1,
    LogLevel_Debug = 2,
    LogLevel_Info = 3,
    LogLevel_Warn = 4,
    LogLevel_Error = 5,
    LogLevel_Fatal = 6,

    LogLevel_Count,
    _LogLevel_Force32 = 0x7FFFFFFF
} LogLevel;

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
    WindowEventType_Enter,
    WindowEventType_Leave,
    WindowEventType_FocusGained,
    WindowEventType_FocusLost,
    WindowEventType_CloseRequested,

    _WindowEventType_Count,
    _WindowEventType_Force32 = 0x7FFFFFFF
} WindowEventType;

typedef enum PixelFormat {
    PixelFormat_Undefined = 0,
    // 8-bit formats
    PixelFormat_R8Unorm,
    PixelFormat_R8Snorm,
    PixelFormat_R8Uint,
    PixelFormat_R8Sint,
    // 16-bit formats
    PixelFormat_R16Unorm,
    PixelFormat_R16Snorm,
    PixelFormat_R16Uint,
    PixelFormat_R16Sint,
    PixelFormat_R16Float,
    PixelFormat_RG8Unorm,
    PixelFormat_RG8Snorm,
    PixelFormat_RG8Uint,
    PixelFormat_RG8Sint,
    // Packed 16-Bit formats
    PixelFormat_B5G6R5Unorm,
    PixelFormat_BGR5A1Unorm,
    PixelFormat_BGRA4Unorm,
    // 32-bit formats
    PixelFormat_R32Uint,
    PixelFormat_R32Sint,
    PixelFormat_R32Float,
    PixelFormat_RG16Unorm,
    PixelFormat_RG16Snorm,
    PixelFormat_RG16Uint,
    PixelFormat_RG16Sint,
    PixelFormat_RG16Float,
    PixelFormat_RGBA8Unorm,
    PixelFormat_RGBA8UnormSrgb,
    PixelFormat_RGBA8Snorm,
    PixelFormat_RGBA8Uint,
    PixelFormat_RGBA8Sint,
    PixelFormat_BGRA8Unorm,
    PixelFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    PixelFormat_RGB10A2Unorm,
    PixelFormat_RGB10A2Uint,
    PixelFormat_RG11B10Ufloat,
    PixelFormat_RGB9E5Ufloat,
    // 64-bit formats
    PixelFormat_RG32Uint,
    PixelFormat_RG32Sint,
    PixelFormat_RG32Float,
    PixelFormat_RGBA16Unorm,
    PixelFormat_RGBA16Snorm,
    PixelFormat_RGBA16Uint,
    PixelFormat_RGBA16Sint,
    PixelFormat_RGBA16Float,
    // 128-bit formats
    PixelFormat_RGBA32Uint,
    PixelFormat_RGBA32Sint,
    PixelFormat_RGBA32Float,
    // Depth-stencil formats
    PixelFormat_Depth16Unorm,
    PixelFormat_Depth24UnormStencil8,
    PixelFormat_Depth32Float,
    PixelFormat_Depth32FloatStencil8,
    // BC compressed formats
    PixelFormat_BC1RGBAUnorm,
    PixelFormat_BC1RGBAUnormSrgb,
    PixelFormat_BC2RGBAUnorm,
    PixelFormat_BC2RGBAUnormSrgb,
    PixelFormat_BC3RGBAUnorm,
    PixelFormat_BC3RGBAUnormSrgb,
    PixelFormat_BC4RUnorm,
    PixelFormat_BC4RSnorm,
    PixelFormat_BC5RGUnorm,
    PixelFormat_BC5RGSnorm,
    PixelFormat_BC6HRGBUfloat,
    PixelFormat_BC6HRGBFloat,
    PixelFormat_BC7RGBAUnorm,
    PixelFormat_BC7RGBAUnormSrgb,
    // ETC2/EAC compressed formats
    PixelFormat_ETC2RGB8Unorm,
    PixelFormat_ETC2RGB8UnormSrgb,
    PixelFormat_ETC2RGB8A1Unorm,
    PixelFormat_ETC2RGB8A1UnormSrgb,
    PixelFormat_ETC2RGBA8Unorm,
    PixelFormat_ETC2RGBA8UnormSrgb,
    PixelFormat_EACR11Unorm,
    PixelFormat_EACR11Snorm,
    PixelFormat_EACRG11Unorm,
    PixelFormat_EACRG11Snorm,
    // ASTC compressed formats
    PixelFormat_ASTC4x4Unorm,
    PixelFormat_ASTC4x4UnormSrgb,
    PixelFormat_ASTC5x4Unorm,
    PixelFormat_ASTC5x4UnormSrgb,
    PixelFormat_ASTC5x5Unorm,
    PixelFormat_ASTC5x5UnormSrgb,
    PixelFormat_ASTC6x5Unorm,
    PixelFormat_ASTC6x5UnormSrgb,
    PixelFormat_ASTC6x6Unorm,
    PixelFormat_ASTC6x6UnormSrgb,
    PixelFormat_ASTC8x5Unorm,
    PixelFormat_ASTC8x5UnormSrgb,
    PixelFormat_ASTC8x6Unorm,
    PixelFormat_ASTC8x6UnormSrgb,
    PixelFormat_ASTC8x8Unorm,
    PixelFormat_ASTC8x8UnormSrgb,
    PixelFormat_ASTC10x5Unorm,
    PixelFormat_ASTC10x5UnormSrgb,
    PixelFormat_ASTC10x6Unorm,
    PixelFormat_ASTC10x6UnormSrgb,
    PixelFormat_ASTC10x8Unorm,
    PixelFormat_ASTC10x8UnormSrgb,
    PixelFormat_ASTC10x10Unorm,
    PixelFormat_ASTC10x10UnormSrgb,
    PixelFormat_ASTC12x10Unorm,
    PixelFormat_ASTC12x10UnormSrgb,
    PixelFormat_ASTC12x12Unorm,
    PixelFormat_ASTC12x12UnormSrgb,
    // ASTC HDR compressed formats
    PixelFormat_ASTC4x4HDR,
    PixelFormat_ASTC5x4HDR,
    PixelFormat_ASTC5x5HDR,
    PixelFormat_ASTC6x5HDR,
    PixelFormat_ASTC6x6HDR,
    PixelFormat_ASTC8x5HDR,
    PixelFormat_ASTC8x6HDR,
    PixelFormat_ASTC8x8HDR,
    PixelFormat_ASTC10x5HDR,
    PixelFormat_ASTC10x6HDR,
    PixelFormat_ASTC10x8HDR,
    PixelFormat_ASTC10x10HDR,
    PixelFormat_ASTC12x10HDR,
    PixelFormat_ASTC12x12HDR,

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _PixelFormat_Count,
    _PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum PixelFormatKind {
    /// Unsigned normalized formats
    PixelFormatKind_Unorm,
    /// Unsigned normalized sRGB formats
    PixelFormatKind_UnormSrgb,
    /// Signed normalized formats
    PixelFormatKind_Snorm,
    /// Unsigned integer formats
    PixelFormatKind_Uint,
    /// Unsigned integer formats
    PixelFormatKind_Sint,
    /// Floating-point formats
    PixelFormatKind_Float,

    _PixelFormatKind_Count,
    _PixelFormatKind_Force32 = 0x7FFFFFFF
} PixelFormatKind;

typedef enum TextureDimension {
    /// Undefined - default to 2D texture.
    TextureDimension_Undefined = 0,
    /// One-dimensional Texture.
    TextureDimension_1D = 1,
    /// Two-dimensional Texture.
    TextureDimension_2D = 2,
    /// Three-dimensional Texture.
    TextureDimension_3D = 3,
    /// Cubemap Texture.
    TextureDimension_Cube = 4,

    _TextureDimension_Force32 = 0x7FFFFFFF
} TextureDimension;

typedef Flags WindowFlags;
static const WindowFlags WindowFlags_None = 0x0000000000000000;
static const WindowFlags WindowFlags_Fullscreen = 0x0000000000000001;
static const WindowFlags WindowFlags_Hidden = 0x0000000000000002;
static const WindowFlags WindowFlags_Borderless = 0x0000000000000004;
static const WindowFlags WindowFlags_Resizable = 0x0000000000000008;
static const WindowFlags WindowFlags_Maximized = 0x0000000000000010;
static const WindowFlags WindowFlags_AlwaysOnTop = 0x0000000000000020;

/* Structs */
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

typedef struct Quaternion {
    float x;
    float y;
    float z;
    float w;
} Quaternion;

typedef struct Color {
    float r;
    float g;
    float b;
    float a;
} Color;

/// 4x4 row-major matrix: 32 bit floating point components
typedef struct Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} Matrix4x4;

typedef struct PixelFormatInfo {
    PixelFormat format;
    const char* name;
    uint8_t bytesPerBlock;
    uint8_t blockWidth;
    uint8_t blockHeight;
    PixelFormatKind kind;
} PixelFormatInfo;

typedef struct Blob {
    uint32_t ref;
    void* data;
    size_t size;
    char* name;
} Blob;

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
    bool alt;
    bool ctrl;
    bool shift;
    bool system;
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

typedef struct ImageLevel {
    uint32_t      width;
    uint32_t      height;
    PixelFormat   format;
    uint32_t      rowPitch;
    uint32_t      slicePitch;
    uint8_t* pixels;
} ImageLevel;

typedef struct ImageDesc {
    TextureDimension    dimension;
    PixelFormat         format;
    uint32_t            width;
    uint32_t            height;
    uint32_t            depthOrArrayLayers;
    uint32_t            mipLevelCount;
} ImageDesc;

/* Platform */
ALIMER_API void alimerGetVersion(uint32_t* major, uint32_t* minor, uint32_t* patch);

/* Memory */
ALIMER_API void* alimerCalloc(size_t count, size_t size);
ALIMER_API void* alimerMalloc(size_t size);
ALIMER_API void* alimerRealloc(void* old, size_t size);
ALIMER_API void alimerFree(void* data);

/* Log */
typedef void (*AlimerLogCallback)(LogCategory category, LogLevel level, const char* message, void* userData);

ALIMER_API LogLevel alimerGetLogLevel(void);
ALIMER_API void alimerSetLogLevel(LogLevel level);
ALIMER_API void alimerSetLogCallback(AlimerLogCallback callback, void* userData);

ALIMER_API void alimerLog(LogCategory category, LogLevel level, const char* message);
ALIMER_API void alimerLogFormat(LogCategory category, LogLevel level, const char* format, ...);
ALIMER_API void alimerLogFatal(LogCategory category, const char* format, ...);
ALIMER_API void alimerLogError(LogCategory category, const char* format, ...);
ALIMER_API void alimerLogWarn(LogCategory category, const char* format, ...);
ALIMER_API void alimerLogInfo(LogCategory category, const char* format, ...);
ALIMER_API void alimerLogDebug(LogCategory category, const char* format, ...);
ALIMER_API void alimerLogTrace(LogCategory category, const char* format, ...);

/* Blog */
ALIMER_API Blob* alimerBlobCreate(void* data, size_t size, const char* name);
ALIMER_API void alimerBlobDestroy(Blob* blob);

/* Platform */
ALIMER_API bool alimerPlatformInit(void);
ALIMER_API void alimerPlatformShutdown(void);
ALIMER_API bool alimerPlatformPollEvent(PlatformEvent* evt);

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

ALIMER_API void* alimerWindowGetNativeHandle(Window* window);

/* Clipboard */
ALIMER_API bool alimerHasClipboardText(void);
ALIMER_API const char* alimerClipboardGetText(void);
ALIMER_API void alimerClipboardSetText(const char* text);

/* PixelFormat */
ALIMER_API void alimerPixelFormatGetInfo(PixelFormat format, PixelFormatInfo* pInfo);
/// Check if the format has a depth component
ALIMER_API bool alimerPixelFormatIsDepth(PixelFormat format);
/// Check if the format has a stencil component
ALIMER_API bool alimerPixelFormatIsStencil(PixelFormat format);
/// Check if the format has depth or stencil components
ALIMER_API bool alimerPixelFormatIsDepthStencil(PixelFormat format);
/// Check if the format has a depth only component.
ALIMER_API bool alimerPixelFormatIsDepthOnly(PixelFormat format);
/// Check if the format is a compressed format.
ALIMER_API bool alimerPixelFormatIsCompressed(PixelFormat format);
/// Check if the format is a BC-compressed format.
ALIMER_API bool alimerPixelFormatIsCompressedBC(PixelFormat format);
/// Check if the format is a ASTC-compressed format.
ALIMER_API bool alimerPixelFormatIsCompressedASTC(PixelFormat format);
/// Get the number of bytes per format.
ALIMER_API uint32_t alimerPixelFormatGetBytesPerBlock(PixelFormat format);
/// Get the pixel format kind
ALIMER_API PixelFormatKind alimerPixelFormatGetKind(PixelFormat format);
/// Check if a format is an integer type.
ALIMER_API bool alimerPixelFormatIsInteger(PixelFormat format);
/// Check if a format represents sRGB color space
ALIMER_API bool alimerPixelFormatIsSrgb(PixelFormat format);

/// Convert an SRGB format to linear. If the format is already linear, will return it
ALIMER_API PixelFormat alimerPixelFormatSrgbToLinear(PixelFormat format);
/// Convert an linear format to sRGB. If the format doesn't have a matching sRGB format, will return the original
ALIMER_API PixelFormat alimerPixelFormatLinearToSrgb(PixelFormat format);

/// Get bits per pixel for a given format
ALIMER_API uint32_t alimerPixelFormatGetBitsPerPixel(PixelFormat format);

/// Get surface information for a given format and dimensions
ALIMER_API void alimerGetSurfaceInfo(PixelFormat format, uint32_t width, uint32_t height, uint32_t* pRowPitch, uint32_t* pSlicePitch, uint32_t* pWidthCount /*= nullptr*/, uint32_t* pHeightCount /*= nullptr*/);

ALIMER_API uint32_t alimerPixelFormatToDxgiFormat(PixelFormat format);
ALIMER_API PixelFormat alimerPixelFormatFromDxgiFormat(uint32_t dxgiFormat);
ALIMER_API uint32_t alimerPixelFormatToVkFormat(PixelFormat format);
ALIMER_API PixelFormat alimerPixelFormatFromVkFormat(uint32_t vkFormat);

/* Image */
ALIMER_API Image* alimerImageCreate1D(PixelFormat format, uint32_t width, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreate2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreate3D(PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreateCube(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API Image* alimerImageCreateFromMemory(const uint8_t* pData, size_t dataSize);
ALIMER_API void alimerImageDestroy(Image* image);

ALIMER_API void alimerImageGetDesc(Image* image, ImageDesc* pDesc);
ALIMER_API TextureDimension alimerImageGetDimension(Image* image);
ALIMER_API PixelFormat alimerImageGetFormat(Image* image);
ALIMER_API uint32_t alimerImageGetWidth(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetHeight(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetDepth(Image* image, uint32_t level);
ALIMER_API uint32_t alimerImageGetArrayLayers(Image* image);
ALIMER_API uint32_t alimerImageGetMipLevelCount(Image* image);
ALIMER_API uint8_t* alimerImageGetPixels(Image* image, size_t* pixelsSize);
ALIMER_API ImageLevel* alimerImageGetLevel(Image* image, uint32_t mipLevel, uint32_t arrayOrDepthSlice /* = 0*/);

/// Save in JPG format to file with specified quality. Return true if successful.
ALIMER_API Blob* alimerImageEncodeJPG(Image* image, int quality);

/* Font */
ALIMER_API Font* alimerFontCreateFromMemory(const uint8_t* data, size_t size);
ALIMER_API void alimerFontDestroy(Font* font);
ALIMER_API void alimerFontGetMetrics(Font* font, int* ascent, int* descent, int* linegap);
ALIMER_API int alimerFontGetGlyphIndex(Font* font, int codepoint);
ALIMER_API float alimerFontGetScale(Font* font, float size);
ALIMER_API float alimerFontGetKerning(Font* font, int glyph1, int glyph2, float scale);
ALIMER_API void alimerFontGetCharacter(Font* font, int glyph, float scale, int* width, int* height, float* advance, float* offsetX, float* offsetY, int* visible);
ALIMER_API void alimerFontGetPixels(Font* font, uint8_t* dest, int glyph, int width, int height, float scale);

#if defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(_MSC_VER)
#   pragma warning(pop)
#endif

#endif /* ALIMER_H_ */
