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
    LogCategory_Application,
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

typedef enum EventType {
    EventType_Unknown = 0,
    EventType_Quit,

    EventType_Window,
    EventType_KeyDown,
    EventType_KeyUp,
    EventType_TextInput,

    EventType_MouseButton,
    EventType_MouseMotion,
    EventType_MouseWheel,

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
    WindowEventType_Close,

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

typedef struct PixelFormatInfo
{
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
    uint8_t*    data;
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
    uint32_t windowId;
    int32_t data1;
    int32_t data2;
} WindowEvent;

typedef struct MouseButtonEvent
{
    uint32_t windowId;
    float x;
    float y;
    MouseButton button;
    ButtonState state;
} MouseButtonEvent;

typedef struct MouseMotionEvent
{
    uint32_t windowId;
    int32_t x;
    int32_t y;
} MouseMotionEvent;

typedef struct MouseWheelEvent
{
    uint32_t windowId;
    float x;
    float y;
} MouseWheelEvent;

typedef struct Event {
    EventType       type;
    union
    {
        WindowEvent window;
        MouseButtonEvent button;
        MouseMotionEvent motion;
        MouseWheelEvent wheel;
    };
} Event;

typedef struct ImageLevel {
    uint32_t    width;
    uint32_t    height;
    uint32_t    rowPitch;
    uint32_t    slicePitch;
    uint8_t*    pixels;
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
ALIMER_API bool alimerPollEvent(Event* evt);

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
ALIMER_API void alimerWindowSetTitle(Window* window, const char* title);
ALIMER_API const char* alimerWindowGetTitle(Window* window);
ALIMER_API bool alimerWindowIsMinimized(Window* window);
ALIMER_API bool alimerWindowHasFocus(Window* window);
ALIMER_API void alimerWindowShow(Window* window);
ALIMER_API void alimerWindowHide(Window* window);
ALIMER_API void alimerWindowMaximize(Window* window);
ALIMER_API void alimerWindowMinimize(Window* window);
ALIMER_API void alimerWindowRestore(Window* window);
ALIMER_API void alimerWindowRaise(Window* window);
ALIMER_API void alimerWindowRequestFocus(Window* window);

ALIMER_API void* alimerWindowGetNativeHandle(Window* window);

/* Clipboard */
ALIMER_API void alimerClipboardSetText(const char* text);
ALIMER_API const char* alimerClipboardGetText(void);

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

ALIMER_API uint32_t alimerPixelFormatToDxgiFormat(PixelFormat format);
ALIMER_API PixelFormat alimerPixelFormatFromDxgiFormat(uint32_t dxgiFormat);
ALIMER_API uint32_t alimerPixelFormatToVkFormat(PixelFormat format);
ALIMER_API PixelFormat alimerPixelFormatFromVkFormat(uint32_t vkFormat);

/* Image */
ALIMER_API Image* alimerImageCreate2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);
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
ALIMER_API void* alimerImageGetData(Image* image, size_t* size);

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
