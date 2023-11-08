// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_H
#define ALIMER_H

#if defined(ALIMER_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define ALIMER_EXPORT __declspec(dllexport)
#        else
#            define ALIMER_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(ALIMER_IMPLEMENTATION)
#            define ALIMER_EXPORT __attribute__((visibility("default")))
#        else
#            define ALIMER_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined(ALIMER_SHARED_LIBRARY)
#    define ALIMER_EXPORT
#endif

#ifdef __cplusplus
#    define _ALIMER_EXTERN extern "C"
#else
#    define _ALIMER_EXTERN extern
#endif

#define ALIMER_API _ALIMER_EXTERN ALIMER_EXPORT 

/* Defines */
#define ALIMER_VERSION_MAJOR	1
#define ALIMER_VERSION_MINOR	0
#define ALIMER_VERSION_PATCH	0

#define ALIMER_MAX_MESSAGE_SIZE 1024

#include <stddef.h>
#include <stdint.h>

/* Types */
typedef uint32_t Flags;
typedef uint32_t Bool32;

/* Enums */

/// Identifiers the running platform type.
typedef enum PlatformID
{
    /// Unknown platform.
    PlatformID_Unknown,
    /// Windows platform.
    PlatformID_Windows,
    /// UniversalWindows platform.
    PlatformID_UWP,
    /// Xbox One platform.
    PlatformID_XboxOne,
    /// Xbox Series X|S platform.
    PlatformID_XboxScarlett,

    /// Linux platform.
    PlatformID_Linux,
    /// Android platform.
    PlatformID_Android,

    /// macOS platform.
    PlatformID_MacOS,
    /// iOS platform.
    PlatformID_iOS,
    /// tvOS platform.
    PlatformID_tvOS,
    /// Web platform.
    PlatformID_Web,

    _PlatformID_Force32 = 0x7FFFFFFF
} PlatformID;

/// Identifiers the running platform family.
typedef enum PlatformFamily
{
    /// Unknown family.
    PlatformFamily_Unknown,
    /// Mobile family.
    PlatformFamily_Mobile,
    /// Desktop family.
    PlatformFamily_Desktop,
    /// Console family.
    PlatformFamily_Console,

    _PlatformFamily_Force32 = 0x7FFFFFFF
} PlatformFamily;

typedef enum LogLevel {
    LogLevel_Trace = 0,
    LogLevel_Debug = 1,
    LogLevel_Info = 2,
    LogLevel_Warn = 3,
    LogLevel_Error = 4,
    LogLevel_Fatal = 5,
    LogLevel_Off = 6,

    _LogLevel_Force32 = 0x7FFFFFFF
} LogLevel;

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
    PixelFormat_BGRA4Unorm,
    PixelFormat_B5G6R5Unorm,
    PixelFormat_BGR5A1Unorm,
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
    PixelFormat_RG11B10UFloat,
    PixelFormat_RGB9E5UFloat,
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

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _PixelFormat_Count,
    _PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum TextureDimension
{
    /// One-dimensional Texture.
    TextureDimension_1D,
    /// Two-dimensional Texture.
    TextureDimension_2D,
    /// Three-dimensional Texture.
    TextureDimension_3D,
    /// Cubemap Texture.
    TextureDimension_Cube,

    _TextureDimension_Count,
    _TextureDimension_Force32 = 0x7FFFFFFF
} TextureDimension;

/* Structs */
typedef struct Vector2
{
    float x;
    float y;
} Vector2;

typedef struct Vector3
{
    float x;
    float y;
    float z;
} Vector3;

/* Methods*/
ALIMER_API void alimerGetVersion(int* major, int* minor, int* patch);
ALIMER_API PlatformID alimerGetPlatformID(void);
ALIMER_API PlatformFamily alimerGetPlatformFamily(void);
ALIMER_API const char* alimerGetPlatformName(void);

/* Log */
typedef void (*AlimerLogCallback)(LogLevel level, const char* message, void* userData);

ALIMER_API LogLevel alimerGetLogLevel(void);
ALIMER_API void alimerSetLogLevel(LogLevel level);
ALIMER_API void alimerSetLogCallback(AlimerLogCallback callback, void* userData);

ALIMER_API void alimerLog(LogLevel level, const char* message);
ALIMER_API void alimerLogInfo(const char* format, ...);
ALIMER_API void alimerLogWarn(const char* format, ...);
ALIMER_API void alimerLogError(const char* format, ...);
ALIMER_API void alimerLogFatal(const char* format, ...);

/* Platform */
typedef enum ButtonState {
    ButtonState_None,
    ButtonState_Pressed,
    ButtonState_Released,

    _ButtonState_Count,
    _ButtonState_Force32 = 0x7FFFFFFF
} ButtonState;

typedef enum MouseButton {
    /// The left mouse button.
    MouseButton_Left,
    /// The middle mouse button.
    MouseButton_Middle,
    /// The right mouse button.
    MouseButton_Right,
    /// The first extended mouse button.
    MouseButton_XButton1,
    /// The second extended mouse button.
    MouseButton_XButton2,

    _MouseButton_Count,
    _MouseButton_Force32 = 0x7FFFFFFF
} MouseButton;

typedef enum EventType {
    EventType_Quit,

    EventType_MouseButton,
    EventType_MouseMotion,
    EventType_MouseWheel,

    _EventType_Quit_Count,
    _EventType_Quit_Force32 = 0x7FFFFFFF
} EventType;

typedef struct MouseButtonEvent
{
    ButtonState state;
    MouseButton button;
    Vector2 position;
} MouseButtonEvent;

typedef struct Event
{
    EventType type;
    union {
        MouseButtonEvent mouseButton;
    };
} Event;

typedef void (*AlimerEventCallback)(const Event* event);

typedef struct Config {
    const char* applicationName;
    AlimerEventCallback onEvent;
    void* userData;
} Config;

ALIMER_API Bool32 Alimer_Init(const Config* config);
ALIMER_API void Alimer_Shutdown(void);

ALIMER_API void Alimer_SetClipboardText(const char* text);
ALIMER_API const char* Alimer_GetClipboardText(void);

/* Image methods */
typedef enum ImageFileFormat {
    ImageFileFormat_Bmp,
    ImageFileFormat_Png,
    ImageFileFormat_Jpg,
    ImageFileFormat_Tga,
    ImageFileFormat_Hdr,

    _ImageFileFormat_Force32 = 0x7FFFFFFF
} ImageFileFormat;

typedef struct ImageDesc {
    TextureDimension dimension;
    PixelFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
    uint32_t mipLevelCount;
} ImageDesc;

typedef struct AlimerImage AlimerImage;
typedef void (*AlimerImageSaveCallback)(AlimerImage* image, void* pData, uint32_t dataSize);

ALIMER_API AlimerImage* AlimerImage_Create2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t arrayLayers, uint32_t mipLevelCount);
ALIMER_API AlimerImage* AlimerImage_CreateFromMemory(const void* data, size_t size);
ALIMER_API void AlimerImage_Destroy(AlimerImage* image);

ALIMER_API void AlimerImage_GetDesc(AlimerImage* image, ImageDesc* pDesc);
ALIMER_API TextureDimension AlimerImage_GetDimension(AlimerImage* image);
ALIMER_API PixelFormat AlimerImage_GetFormat(AlimerImage* image);
ALIMER_API uint32_t AlimerImage_GetWidth(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetHeight(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetDepth(AlimerImage* image, uint32_t level);
ALIMER_API uint32_t AlimerImage_GetArrayLayers(AlimerImage* image);
ALIMER_API uint32_t AlimerImage_GetMipLevelCount(AlimerImage* image);
ALIMER_API void* AlimerImage_GetData(AlimerImage* image, size_t* size);
ALIMER_API Bool32 AlimerImage_Save(AlimerImage* image, ImageFileFormat format, int quality, AlimerImageSaveCallback callback);

/* Font */
typedef struct AlimerFont AlimerFont;

ALIMER_API AlimerFont* AlimerFont_CreateFromMemory(const uint8_t* data, size_t size);
ALIMER_API void AlimerFont_Destroy(AlimerFont* font);
ALIMER_API void AlimerFont_GetMetrics(AlimerFont* font, int* ascent, int* descent, int* linegap);
ALIMER_API int AlimerFont_GetGlyphIndex(AlimerFont* font, int codepoint);
ALIMER_API float AlimerFont_GetScale(AlimerFont* font, float size);
ALIMER_API float AlimerFont_GetKerning(AlimerFont* font, int glyph1, int glyph2, float scale);
ALIMER_API void AlimerFont_GetCharacter(AlimerFont* font, int glyph, float scale, int* width, int* height, float* advance, float* offsetX, float* offsetY, int* visible);
ALIMER_API void AlimerFont_GetPixels(AlimerFont* font, uint8_t* dest, int glyph, int width, int height, float scale);

/* Audio */
typedef enum SoundFlags
{
    SoundFlags_None = 0,
    SoundFlags_Stream = 0x00000001,
    SoundFlags_Decode = 0x00000002,
    SoundFlags_Async = 0x00000004,
    SoundFlags_WaitInit = 0x00000008,

    SoundFlags_NoDefaultAttachment = 0x00001000,
    SoundFlags_NoPitch = 0x00002000,
    SoundFlags_NoSpatialization = 0x00004000,

    _SoundFlags_Force32 = 0x7FFFFFFF
} SoundFlags;

typedef struct AudioConfig {
    uint32_t listenerCount;
    uint32_t channels;
    uint32_t sampleRate;
} AudioConfig;

typedef struct AlimerSound AlimerSound;

ALIMER_API Bool32 Alimer_AudioInit(const AudioConfig* config);
ALIMER_API void Alimer_AudioShutdown(void);
ALIMER_API Bool32 Alimer_AudioStart(void);
ALIMER_API Bool32 Alimer_AudioStop(void);

ALIMER_API uint32_t Alimer_AudioGetOutputChannels(void);
ALIMER_API uint32_t Alimer_AudioGetOutputSampleRate(void);

ALIMER_API float Alimer_AudioGetMasterVolume(void);
ALIMER_API Bool32 Alimer_AudioSetMasterVolume(float volume);

ALIMER_API AlimerSound* AlimerSound_Create(const char* path, uint32_t flags);
ALIMER_API void AlimerSound_Destroy(AlimerSound* sound);
ALIMER_API void AlimerSound_Play(AlimerSound* sound);
ALIMER_API void AlimerSound_Stop(AlimerSound* sound);
ALIMER_API float AlimerSound_GetVolume(AlimerSound* sound);
ALIMER_API void AlimerSound_SetVolume(AlimerSound* sound, float value);
ALIMER_API float AlimerSound_GetPitch(AlimerSound* sound);
ALIMER_API void AlimerSound_SetPitch(AlimerSound* sound, float value);
ALIMER_API float AlimerSound_GetPan(AlimerSound* sound);
ALIMER_API void AlimerSound_SetPan(AlimerSound* sound, float value);

ALIMER_API Bool32 AlimerSound_IsPlaying(AlimerSound* sound);
ALIMER_API Bool32 AlimerSound_GetFinished(AlimerSound* sound);

ALIMER_API uint64_t AlimerSound_GetLengthPcmFrames(AlimerSound* sound);
ALIMER_API uint64_t AlimerSound_GetCursorPcmFrames(AlimerSound* sound);
ALIMER_API Bool32 AlimerSound_SetCursorPcmFrames(AlimerSound* sound, uint64_t value);

ALIMER_API Bool32 AlimerSound_IsLooping(AlimerSound* sound);
ALIMER_API void AlimerSound_SetLooping(AlimerSound* sound, Bool32 value);
ALIMER_API void AlimerSound_GetLoopPcmFrames(AlimerSound* sound, uint64_t* pLoopBegInFrames, uint64_t* pLoopEndInFrames);
ALIMER_API Bool32 AlimerSound_SetLoopPcmFrames(AlimerSound* sound, uint64_t loopBegInFrames, uint64_t loopEndInFrames);

ALIMER_API Bool32 AlimerSound_IsSpatialized(AlimerSound* sound);
ALIMER_API void AlimerSound_SetSpatialized(AlimerSound* sound, Bool32 value);

ALIMER_API void AlimerSound_GetPosition(AlimerSound* sound, Vector3* result);
ALIMER_API void AlimerSound_SetPosition(AlimerSound* sound, Vector3* value);
ALIMER_API void AlimerSound_GetVelocity(AlimerSound* sound, Vector3* result);
ALIMER_API void AlimerSound_SetVelocity(AlimerSound* sound, Vector3* value);
ALIMER_API void AlimerSound_GetDirection(AlimerSound* sound, Vector3* result);
ALIMER_API void AlimerSound_SetDirection(AlimerSound* sound, Vector3* value);

/* GPU */
typedef struct GPUBuffer GPUBuffer;
typedef struct GPUTexture GPUTexture;

#endif /* ALIMER_H */
