// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_H_
#define ALIMER_H_ 1

#include "alimer_types.h"

/* Version API */
#define ALIMER_VERSION_MAJOR    1
#define ALIMER_VERSION_MINOR    0
#define ALIMER_VERSION_PATCH	0

#define MAX_LOG_MESSAGE_SIZE        1024

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

/* Structs */
typedef struct PixelFormatInfo {
    PixelFormat format;
    const char* name;
    uint8_t bytesPerBlock;
    uint8_t blockWidth;
    uint8_t blockHeight;
    PixelFormatKind kind;
} PixelFormatInfo;

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
ALIMER_API uint32_t alimerVkFormatFromOpenGLInternalFormat(uint32_t glInternalformat);

#endif /* ALIMER_H_ */
