// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_gpu_internal.h"
#include <stdio.h>
#include <stdarg.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#define MAX_MESSAGE_SIZE 1024

static GPULogLevel s_LogLevel = GPULogLevel_Off;
static GPULogCallback s_LogFunc = nullptr;
static void* s_userData = nullptr;

GPULogLevel agpuGetLogLevel(void)
{
    return s_LogLevel;
}

void agpuSetLogLevel(GPULogLevel level)
{
    s_LogLevel = level;
}

void agpuSetLogCallback(GPULogCallback func, void* userData)
{
    s_LogFunc = func;
    s_userData = userData;
}

bool agpuShouldLog(GPULogLevel level)
{
    if (s_LogLevel == GPULogLevel_Off || s_LogFunc == nullptr)
        return false;

    return level <= s_LogLevel;
}

void agpuLogInfo(const char* format, ...)
{
    if (!agpuShouldLog(GPULogLevel_Info))
        return;

    char msg[MAX_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, MAX_MESSAGE_SIZE, format, args);
    va_end(args);

    s_LogFunc(GPULogLevel_Info, msg, s_userData);
}

void agpuLogWarn(const char* format, ...)
{
    if (!agpuShouldLog(GPULogLevel_Warn))
        return;

    char msg[MAX_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, MAX_MESSAGE_SIZE, format, args);
    va_end(args);
    s_LogFunc(GPULogLevel_Warn, msg, s_userData);
}

void agpuLogError(const char* format, ...)
{
    if (!agpuShouldLog(GPULogLevel_Error))
        return;

    char msg[MAX_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, MAX_MESSAGE_SIZE, format, args);
    va_end(args);

    s_LogFunc(GPULogLevel_Error, msg, s_userData);
}



/* PixelFormat */
// Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
static const GPUPixelFormatInfo kPixelFormatInfo[] = {
    //        format                   name             bytes blk         kind               red   green   blue  alpha  depth  stencl signed  srgb
    { GPUPixelFormat_Undefined,        "Undefined",          0,   0, 0,  _GPUPixelFormatKind_Force32 },
    // 8-bit formats
    { GPUPixelFormat_R8Unorm,          "R8Unorm",            1,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_R8Snorm,          "R8Snorm",            1,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_R8Uint,           "R8Uint",             1,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_R8Sint,           "R8Sint",             1,   1, 1, GPUPixelFormatKind_Sint },
    // 16-bit formats
    { GPUPixelFormat_R16Unorm,         "R16Unorm",         2,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_R16Snorm,         "R16Snorm",         2,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_R16Uint,          "R16Uint",          2,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_R16Sint,          "R16Sint",          2,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_R16Float,         "R16Float",         2,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_RG8Unorm,         "RG8Unorm",         2,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_RG8Snorm,         "RG8Snorm",         2,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_RG8Uint,          "RG8Uint",          2,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RG8Sint,          "RG8Sint",          2,   1, 1, GPUPixelFormatKind_Sint },
    // Packed 16-Bit formats
    { GPUPixelFormat_B5G6R5Unorm,      "B5G6R5Unorm",      2,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BGR5A1Unorm,      "BGR5A1Unorm",      2,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BGRA4Unorm,       "BGRA4Unorm",       2,   1, 1, GPUPixelFormatKind_Unorm },
    // 32-bit formats
    { GPUPixelFormat_R32Uint,          "R32Uint",          4,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_R32Sint,          "R32Sint",          4,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_R32Float,         "R32Float",         4,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_RG16Unorm,        "RG16Unorm",        4,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_RG16Snorm,        "RG16Snorm",        4,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_RG16Uint,         "RG16Uint",         4,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RG16Sint,         "RG16Sint",         4,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_RG16Float,        "RG16Float",        4,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_RGBA8Unorm,       "RGBA8Unorm",       4,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_RGBA8UnormSrgb,   "RGBA8UnormSrgb",   4,   1, 1, GPUPixelFormatKind_UnormSrgb  },
    { GPUPixelFormat_RGBA8Snorm,       "RGBA8Snorm",       4,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_RGBA8Uint,        "RGBA8Uint",        4,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RGBA8Sint,        "RGBA8Sint",        4,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_BGRA8Unorm,       "BGRA8Unorm",       4,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BGRA8UnormSrgb,   "BGRA8UnormSrgb",   4,   1, 1, GPUPixelFormatKind_UnormSrgb },
    // Packed 32-Bit Pixel Formats
    { GPUPixelFormat_RGB10A2Unorm,    "RGB10A2Unorm",      4,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_RGB10A2Uint,     "RGB10A2Uint",       4,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RG11B10Ufloat,   "RG11B10Ufloat",     4,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_RGB9E5Ufloat,    "RGB9E5Ufloat",      4,   1, 1, GPUPixelFormatKind_Float },
    // 64-bit formats
    { GPUPixelFormat_RG32Uint,         "RG32Uint",         8,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RG32Sint,         "RG32Sint",         8,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_RG32Float,        "RG32Float",        8,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_RGBA16Unorm,      "RGBA16Unorm",      8,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_RGBA16Snorm,      "RGBA16Snorm",      8,   1, 1, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_RGBA16Uint,       "RGBA16Uint",       8,   1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RGBA16Sint,       "RGBA16Sint",       8,   1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_RGBA16Float,      "RGBA16Float",      8,   1, 1, GPUPixelFormatKind_Float },
    // 128-bit formats
    { GPUPixelFormat_RGBA32Uint,       "RGBA32Uint",       16,  1, 1, GPUPixelFormatKind_Uint },
    { GPUPixelFormat_RGBA32Sint,       "RGBA32Sint",       16,  1, 1, GPUPixelFormatKind_Sint },
    { GPUPixelFormat_RGBA32Float,      "RGBA32Float",      16,  1, 1, GPUPixelFormatKind_Float },
    // Depth-stencil formats
    //{ GPUPixelFormat_Stencil8,               "Stencil8",                 4,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_Depth16Unorm,           "Depth16Unorm",             2,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_Depth24UnormStencil8,   "Depth24UnormStencil8",     4,   1, 1, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_Depth32Float,           "Depth32Float",             4,   1, 1, GPUPixelFormatKind_Float },
    { GPUPixelFormat_Depth32FloatStencil8,   "Depth32FloatStencil8",     8,   1, 1, GPUPixelFormatKind_Float },
    // BC compressed formats
    { GPUPixelFormat_BC1RGBAUnorm,       "BC1RGBAUnorm",         8,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC1RGBAUnormSrgb,   "BC1RGBAUnormSrgb",     8,   4, 4, GPUPixelFormatKind_UnormSrgb  },
    { GPUPixelFormat_BC2RGBAUnorm,       "BC2RGBAUnorm",         16,  4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC2RGBAUnormSrgb,   "BC2RGBAUnormSrgb",     16,  4, 4, GPUPixelFormatKind_UnormSrgb  },
    { GPUPixelFormat_BC3RGBAUnorm,       "BC3RGBAUnorm",         16,  4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC3RGBAUnormSrgb,   "BC3RGBAUnormSrgb",     16,  4, 4, GPUPixelFormatKind_UnormSrgb  },
    { GPUPixelFormat_BC4RUnorm,          "BC4RUnorm",            8,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC4RSnorm,          "BC4RSnorm",            8,   4, 4, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_BC5RGUnorm,         "BC5Unorm",             16,  4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC5RGSnorm,         "BC5Snorm",             16,  4, 4, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_BC6HRGBUfloat,      "BC6HRGBUfloat",        16,  4, 4, GPUPixelFormatKind_Float },
    { GPUPixelFormat_BC6HRGBFloat,       "BC6HRGBFloat",         16,  4, 4, GPUPixelFormatKind_Float },
    { GPUPixelFormat_BC7RGBAUnorm,       "BC7RGBAUnorm",         16,  4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_BC7RGBAUnormSrgb,   "BC7RGBAUnormSrgb",     16,  4, 4, GPUPixelFormatKind_UnormSrgb },
    // ETC2/EAC compressed formats
    { GPUPixelFormat_ETC2RGBA8Unorm,      "ETC2RGBA8Unorm",       8,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ETC2RGBA8UnormSrgb,  "ETC2RGBA8UnormSrgb",   8,   4, 4, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ETC2RGB8A1Unorm,     "ETC2RGB8A1Unorm,",     16,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ETC2RGB8A1UnormSrgb, "ETC2RGB8A1UnormSrgb",  16,   4, 4, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ETC2RGBA8Unorm,      "ETC2RGBA8Unorm",       16,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ETC2RGBA8UnormSrgb,  "ETC2RGBA8UnormSrgb",   16,   4, 4, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_EACR11Unorm,         "EACR11Unorm",          8,    4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_EACR11Snorm,         "EACR11Snorm",          8,    4, 4, GPUPixelFormatKind_Snorm },
    { GPUPixelFormat_EACRG11Unorm,        "EACRG11Unorm",         16,   4, 4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_EACRG11Snorm,        "EACRG11Snorm",         16,   4, 4, GPUPixelFormatKind_Snorm },
    // ASTC compressed formats
    { GPUPixelFormat_ASTC4x4Unorm,        "ASTC4x4Unorm",         16,   4,   4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC4x4UnormSrgb,    "ASTC4x4UnormSrgb",     16,   4,   4, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC5x4Unorm,        "ASTC5x4Unorm",         16,   5,   4, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC5x4UnormSrgb,    "ASTC5x4UnormSrgb",     16,   5,   4, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC5x5Unorm,        "ASTC5x5UnormSrgb",     16,   5,   5, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC5x5UnormSrgb,    "ASTC5x5UnormSrgb",     16,   5,   5, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC6x5Unorm,        "ASTC6x5Unorm",         16,   6,   5, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC6x5UnormSrgb,    "ASTC6x5UnormSrgb",     16,   6,   5, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC6x6Unorm,        "ASTC6x6Unorm",         16,   6,   6, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC6x6UnormSrgb,    "ASTC6x6UnormSrgb",     16,   6,   6, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC8x5Unorm,        "ASTC8x5Unorm",         16,   8,   5, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC8x5UnormSrgb,    "ASTC8x5UnormSrgb",     16,   8,   5, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC8x6Unorm,        "ASTC8x6Unorm",         16,   8,   6, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC8x6UnormSrgb,    "ASTC8x6UnormSrgb",     16,   8,   6, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC8x8Unorm,        "ASTC8x8Unorm",         16,   8,   8, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC8x8UnormSrgb,    "ASTC8x8UnormSrgb",     16,   8,   8, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC10x5Unorm,       "ASTC10x5Unorm",        16,   10,  5, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC10x5UnormSrgb,   "ASTC10x5UnormSrgb",    16,   10,  5, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC10x6Unorm,       "ASTC10x6Unorm",        16,   10,  6, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC10x6UnormSrgb,   "ASTC10x6UnormSrgb",    16,   10,  6, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC10x8Unorm,       "ASTC10x8Unorm",        16,   10,  8, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC10x8UnormSrgb,   "ASTC10x8UnormSrgb",    16,   10,  8, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC10x10Unorm,      "ASTC10x10Unorm",       16,   10,  10, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC10x10UnormSrgb,  "ASTC10x10UnormSrgb",   16,   10,  10, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC12x10Unorm,      "ASTC12x10Unorm",       16,   12,  10, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC12x10UnormSrgb,  "ASTC12x10UnormSrgb",   16,   12,  10, GPUPixelFormatKind_UnormSrgb },
    { GPUPixelFormat_ASTC12x12Unorm,      "ASTC12x12Unorm",       16,   12,  12, GPUPixelFormatKind_Unorm },
    { GPUPixelFormat_ASTC12x12UnormSrgb,  "ASTC12x12UnormSrgb",   16,   12,  12, GPUPixelFormatKind_UnormSrgb },
    // ASTC HDR compressed formats
    { GPUPixelFormat_ASTC4x4HDR,          "ASTC4x4HDR",           16,   4, 4,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC5x4HDR,          "ASTC5x4HDR",           16,   5, 4,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC5x5HDR,          "ASTC5x5HDR",           16,   5, 5,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC6x5HDR,          "ASTC6x5HDR",           16,   6, 5,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC6x6HDR,          "ASTC6x6HDR",           16,   6, 6,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC8x5HDR,          "ASTC8x5HDR",           16,   8, 5,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC8x6HDR,          "ASTC8x6HDR",           16,   8, 6,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC8x8HDR,          "ASTC8x8HDR",           16,   8, 8,    GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC10x5HDR,         "ASTC10x5HDR",          16,   10, 5,   GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC10x6HDR,         "ASTC10x6HDR",          16,   10, 6,   GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC10x8HDR,         "ASTC10x8HDR",          16,   10, 8,   GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC10x10HDR,        "ASTC10x10HDR",         16,   10, 10,  GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC12x10HDR,        "ASTC12x10HDR",         16,   12, 10,  GPUPixelFormatKind_Float },
    { GPUPixelFormat_ASTC12x12HDR,        "ASTC12x12HDR",         16,   12, 12,  GPUPixelFormatKind_Float },
};

static_assert(
    sizeof(kPixelFormatInfo) / sizeof(GPUPixelFormatInfo) == size_t(_GPUPixelFormat_Count),
    "The format info table doesn't have the right number of elements"
    );


GPUPixelFormatInfo agpuPixelFormatGetInfo(GPUPixelFormat format)
{
    ALIMER_ASSERT(size_t(format) < size_t(_GPUPixelFormat_Count));
    ALIMER_ASSERT(kPixelFormatInfo[size_t(format)].format == format);

    return kPixelFormatInfo[size_t(format)];
}

bool agpuIsBackendSupport(GPUBackendType backend)
{
    switch (backend)
    {
        case GPUBackendType_Null:
            return true;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            return Vulkan_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_D3D12:
#if defined(ALIMER_GPU_D3D12)
            return D3D12_IsSupported();
#else
            return false;
#endif

        case GPUBackendType_Metal:
            return false;

        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            return WGPU_IsSupported();
#else
            return false;
#endif

        default:
            return false;
    }
}

GPUFactory agpuCreateFactory(const GPUFactoryDesc* desc)
{
    GPUBackendType backend = (desc != nullptr ? desc->preferredBackend : GPUBackendType_Undefined);
    if (backend == GPUBackendType_Undefined)
    {
        if (agpuIsBackendSupport(GPUBackendType_D3D12))
        {
            backend = GPUBackendType_D3D12;
        }
        else if (agpuIsBackendSupport(GPUBackendType_Metal))
        {
            backend = GPUBackendType_Metal;
        }
        else if (agpuIsBackendSupport(GPUBackendType_Vulkan))
        {
            backend = GPUBackendType_Vulkan;
        }
        else if (agpuIsBackendSupport(GPUBackendType_WebGPU))
        {
            backend = GPUBackendType_WebGPU;
        }
    }

    GPUFactoryImpl* factory = nullptr;
    switch (backend)
    {
        case GPUBackendType_Null:
            factory = Null_CreateFactory(desc);
            break;

        case GPUBackendType_Vulkan:
#if defined(ALIMER_GPU_VULKAN)
            if (Vulkan_IsSupported())
            {
                factory = Vulkan_CreateFactory(desc);
            }
            break;
#else
            agpuLogError("Vulkan is not supported");
            return nullptr;
#endif

        case GPUBackendType_D3D12:
#if defined(ALIMER_GPU_D3D12)
            if (D3D12_IsSupported())
            {
                factory = D3D12_CreateFactory(desc);
            }
            break;
#else
            agpuLogError("D3D12 is not supported");
            return nullptr;
#endif
            break;

        case GPUBackendType_Metal:
            factory = nullptr;
            break;

        case GPUBackendType_WebGPU:
#if defined(ALIMER_GPU_WEBGPU)
            if (WGPU_IsSupported())
            {
                factory = WGPU_CreateInstance(desc);
            }
            break;
#else
            agpuLogError("WebGPU is not supported");
            return nullptr;
#endif

        default:
            break;
    }

    return factory;
}

void agpuFactoryDestroy(GPUFactory factory)
{
    factory->Release();
}

GPUBackendType agpuFactoryGetBackend(GPUFactory factory)
{
    return factory->GetBackend();
}

uint32_t agpuFactoryGetAdapterCount(GPUFactory factory)
{
    return factory->GetAdapterCount();
}

GPUAdapter agpuFactoryGetAdapter(GPUFactory factory, uint32_t index)
{
    return factory->GetAdapter(index);
}

GPUAdapter agpuFactoryGetBestAdapter(GPUFactory factory)
{
    GPUAdapter result = nullptr;
    uint32_t kind = (uint32_t)GPUAdapterType_Other + 1;

    for (uint32_t i = 0, count = factory->GetAdapterCount(); i < count; ++i)
    {
        GPUAdapter adapter = factory->GetAdapter(i);
        GPUAdapterType adapterType = adapter->GetType();
        if ((uint32_t)adapterType < kind)
        {
            result = adapter;
            kind = (uint32_t)adapterType;
        }
    }

    return result;
}

/* Adapter */
void agpuAdapterGetInfo(GPUAdapter adapter, GPUAdapterInfo* info)
{
    if (!info)
        return;

    adapter->GetInfo(info);
}

void agpuAdapterGetLimits(GPUAdapter adapter, GPUAdapterLimits* limits)
{
    if (!limits)
        return;

    adapter->GetLimits(limits);
}

bool agpuAdapterHasFeature(GPUAdapter adapter, GPUFeature feature)
{
    return adapter->HasFeature(feature);
}

/* SurfaceHandle */
GPUSurfaceHandle* agpuSurfaceHandleCreateFromWin32(void* hwnd)
{
    GPUSurfaceHandle* handle = new GPUSurfaceHandle();
    handle->type = GPUSurfaceHandle::Type::WindowsHWND;
    handle->hwnd = static_cast<HWND>(hwnd);
    return handle;
}

GPUSurfaceHandle* agpuSurfaceHandleCreateFromAndroid(void* window)
{
    GPUSurfaceHandle* handle = new GPUSurfaceHandle();
    handle->type = GPUSurfaceHandle::Type::AndroidWindow;
    handle->androidNativeWindow = static_cast<ANativeWindow*>(window);
    return handle;
}

void agpuSurfaceHandleDestroy(GPUSurfaceHandle* surfaceHandle)
{
    delete surfaceHandle;
}

/* Surface */
GPUSurface agpuCreateSurface(GPUFactory factory, GPUSurfaceHandle* handle)
{
    ALIMER_ASSERT(factory);
    ALIMER_ASSERT(handle);

    return factory->CreateSurface(handle);
}

void agpuSurfaceGetCapabilities(GPUSurface surface, GPUAdapter adapter, GPUSurfaceCapabilities* capabilities)
{
    if (!surface || !adapter || !capabilities)
        return;

    surface->GetCapabilities(adapter, capabilities);
}

static GPUSurfaceConfig _GPUSurfaceConfig_Defaults(const GPUSurfaceConfig* config) {
    GPUSurfaceConfig def = *config;
    def.width = _ALIMER_DEF(def.width, 1u);
    def.height = _ALIMER_DEF(def.height, 1u);
    def.presentMode = _ALIMER_DEF(def.presentMode, GPUPresentMode_Fifo);
    return def;
}

bool agpuSurfaceConfigure(GPUSurface surface, const GPUSurfaceConfig* config)
{
    if (!config)
        return false;

    if (!config->device)
    {
        agpuLogError("Surface configuration requires a valid GPUDevice");
        return false;
    }

    GPUSurfaceConfig configDef = _GPUSurfaceConfig_Defaults(config);
    return surface->Configure(&configDef);
}

void agpuSurfaceUnconfigure(GPUSurface surface)
{
    surface->Unconfigure();
}

uint32_t agpuSurfaceAddRef(GPUSurface surface)
{
    return surface->AddRef();
}

uint32_t agpuSurfaceRelease(GPUSurface surface)
{
    return surface->Release();
}

/* Device */
static GPUDeviceDesc _GPUDeviceDesc_Defaults(const GPUDeviceDesc* desc)
{
    GPUDeviceDesc def = {};
    if (desc != nullptr)
        def = *desc;

    // 2 or 3
    def.maxFramesInFlight = std::min(_ALIMER_DEF(def.maxFramesInFlight, 2u), 3u);
    return def;
}

GPUDevice* agpuCreateDevice(GPUAdapter adapter, const GPUDeviceDesc* desc)
{
    GPUDeviceDesc descDef = _GPUDeviceDesc_Defaults(desc);
    return adapter->CreateDevice(descDef);
}

void agpuDeviceSetLabel(GPUDevice* device, const char* label)
{
    device->SetLabel(label);
}

uint32_t agpuDeviceAddRef(GPUDevice* device)
{
    return device->AddRef();
}

uint32_t agpuDeviceRelease(GPUDevice* device)
{
    return device->Release();
}

bool agpuDeviceHasFeature(GPUDevice* device, GPUFeature feature)
{
    return device->HasFeature(feature);
}

GPUCommandQueue* agpuDeviceGetCommandQueue(GPUDevice* device, GPUCommandQueueType type)
{
    return device->GetQueue(type);
}

void agpuDeviceWaitIdle(GPUDevice* device)
{
    device->WaitIdle();
}

uint64_t agpuDeviceGetTimestampFrequency(GPUDevice* device)
{
    return device->GetTimestampFrequency();
}

uint64_t agpuDeviceCommitFrame(GPUDevice* device)
{
    return device->CommitFrame();
}

/* CommandQueue */
GPUCommandQueueType agpuCommandQueueGetType(GPUCommandQueue* queue)
{
    return queue->GetType();
}

void agpuCommandQueueWaitIdle(GPUCommandQueue* queue)
{
    queue->WaitIdle();
}

GPUCommandBuffer* agpuCommandQueueAcquireCommandBuffer(GPUCommandQueue* queue, const GPUCommandBufferDesc* desc)
{
    return queue->AcquireCommandBuffer(desc);
}

void agpuCommandQueueSubmit(GPUCommandQueue* queue, uint32_t numCommandBuffers, GPUCommandBuffer** commandBuffers)
{
    queue->Submit(numCommandBuffers, commandBuffers);
}

/* CommandBuffer */
void agpuCommandBufferPushDebugGroup(GPUCommandBuffer* commandBuffer, const char* groupLabel)
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void agpuCommandBufferPopDebugGroup(GPUCommandBuffer* commandBuffer)
{
    commandBuffer->PopDebugGroup();
}

void agpuCommandBufferInsertDebugMarker(GPUCommandBuffer* commandBuffer, const char* markerLabel)
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

GPUAcquireSurfaceResult agpuCommandBufferAcquireSurfaceTexture(GPUCommandBuffer* commandBuffer, GPUSurface surface, GPUTexture** surfaceTexture)
{
    return commandBuffer->AcquireSurfaceTexture(surface, surfaceTexture);
}

GPUComputePassEncoder* agpuCommandBufferBeginComputePass(GPUCommandBuffer* commandBuffer, const GPUComputePassDesc* desc)
{
    GPUComputePassDesc descDef = {};
    if (desc)
        descDef = *desc;

    return commandBuffer->BeginComputePass(descDef);
}

GPURenderPassEncoder* agpuCommandBufferBeginRenderPass(GPUCommandBuffer* commandBuffer, const GPURenderPassDesc* desc)
{
    if (!desc)
    {
        agpuLogError("Invalid RenderPass description");
        return nullptr;
    }

    return commandBuffer->BeginRenderPass(*desc);
}

/* ComputePassEncoder */
void agpuComputePassEncoderSetPipeline(GPUComputePassEncoder* computePassEncoder, GPUComputePipeline pipeline)
{
    computePassEncoder->SetPipeline(pipeline);
}

void agpuComputePassEncoderSetPushConstants(GPUComputePassEncoder* computePassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size)
{
    computePassEncoder->SetPushConstants(pushConstantIndex, data, size);
}

void agpuComputePassEncoderDispatch(GPUComputePassEncoder* computePassEncoder, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    computePassEncoder->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void agpuComputePassEncoderDispatchIndirect(GPUComputePassEncoder* computePassEncoder, GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    computePassEncoder->DispatchIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuComputePassEncoderEnd(GPUComputePassEncoder* computePassEncoder)
{
    computePassEncoder->EndEncoding();
}

void agpuComputePassEncoderPushDebugGroup(GPUComputePassEncoder* computePassEncoder, const char* groupLabel)
{
    computePassEncoder->PushDebugGroup(groupLabel);
}

void agpuComputePassEncoderPopDebugGroup(GPUComputePassEncoder* computePassEncoder)
{
    computePassEncoder->PopDebugGroup();
}

void agpuComputePassEncoderInsertDebugMarker(GPUComputePassEncoder* computePassEncoder, const char* markerLabel)
{
    computePassEncoder->InsertDebugMarker(markerLabel);
}

/* RenderCommandEncoder */
void agpuRenderPassEncoderSetViewport(GPURenderPassEncoder* renderPassEncoder, const GPUViewport* viewport)
{
    ALIMER_ASSERT(viewport != nullptr);

    renderPassEncoder->SetViewport(viewport);
}

void agpuRenderPassEncoderSetViewports(GPURenderPassEncoder* renderPassEncoder, uint32_t viewportCount, const GPUViewport* viewports)
{
    ALIMER_ASSERT(viewportCount > 0);
    ALIMER_ASSERT(viewports != nullptr);

    renderPassEncoder->SetViewports(viewportCount, viewports);
}

void agpuRenderPassEncoderSetScissorRect(GPURenderPassEncoder* renderPassEncoder, const GPUScissorRect* scissorRect)
{
    ALIMER_ASSERT(scissorRect != nullptr);

    renderPassEncoder->SetScissorRect(scissorRect);
}

void agpuRenderPassEncoderSetScissorRects(GPURenderPassEncoder* renderPassEncoder, uint32_t scissorCount, const GPUScissorRect* scissorRects)
{
    ALIMER_ASSERT(scissorCount > 0);
    ALIMER_ASSERT(scissorRects != nullptr);

    renderPassEncoder->SetScissorRects(scissorCount, scissorRects);
}

void agpuRenderPassEncoderSetBlendColor(GPURenderPassEncoder* renderPassEncoder, const GPUColor* color)
{
    renderPassEncoder->SetBlendColor(color);
}

void agpuRenderPassEncoderSetStencilReference(GPURenderPassEncoder* renderPassEncoder, uint32_t reference)
{
    renderPassEncoder->SetStencilReference(reference);
}

void agpuRenderPassEncoderSetVertexBuffer(GPURenderPassEncoder* renderPassEncoder, uint32_t slot, GPUBuffer* buffer, uint64_t offset)
{
    renderPassEncoder->SetVertexBuffer(slot, buffer, offset);
}

void agpuRenderPassEncoderSetIndexBuffer(GPURenderPassEncoder* renderPassEncoder, GPUBuffer* buffer, GPUIndexType type, uint64_t offset)
{
    renderPassEncoder->SetIndexBuffer(buffer, type, offset);
}

void agpuRenderPassEncoderSetPipeline(GPURenderPassEncoder* renderPassEncoder, GPURenderPipeline pipeline)
{
    renderPassEncoder->SetPipeline(pipeline);
}

void agpuRenderPassEncoderSetPushConstants(GPURenderPassEncoder* renderPassEncoder, uint32_t pushConstantIndex, const void* data, uint32_t size)
{
    renderPassEncoder->SetPushConstants(pushConstantIndex, data, size);
}

void agpuRenderPassEncoderDraw(GPURenderPassEncoder* renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    renderPassEncoder->Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void agpuRenderPassEncoderDrawIndexed(GPURenderPassEncoder* renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance)
{
    renderPassEncoder->DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}

void agpuRenderPassEncoderDrawIndirect(GPURenderPassEncoder* renderPassEncoder, GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    renderPassEncoder->DrawIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuRenderPassEncoderDrawIndexedIndirect(GPURenderPassEncoder* renderPassEncoder, GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset)
{
    renderPassEncoder->DrawIndexedIndirect(indirectBuffer, indirectBufferOffset);
}

void agpuRenderPassEncoderMultiDrawIndirect(GPURenderPassEncoder* renderPassEncoder, GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer, uint64_t drawCountBufferOffset)
{
    renderPassEncoder->MultiDrawIndirect(indirectBuffer, indirectBufferOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

void agpuRenderPassEncoderMultiDrawIndexedIndirect(GPURenderPassEncoder* renderPassEncoder, GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer, uint64_t drawCountBufferOffset)
{
    renderPassEncoder->MultiDrawIndexedIndirect(indirectBuffer, indirectBufferOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
}

void agpuRenderPassEncoderSetShadingRate(GPURenderPassEncoder* renderPassEncoder, GPUShadingRate rate)
{
    renderPassEncoder->SetShadingRate(rate);
}

void agpuRenderPassEncoderEnd(GPURenderPassEncoder* renderPassEncoder)
{
    renderPassEncoder->EndEncoding();
}

void agpuRenderPassEncoderPushDebugGroup(GPURenderPassEncoder* renderPassEncoder, const char* groupLabel)
{
    renderPassEncoder->PushDebugGroup(groupLabel);
}

void agpuRenderPassEncoderPopDebugGroup(GPURenderPassEncoder* renderPassEncoder)
{
    renderPassEncoder->PopDebugGroup();
}

void agpuRenderPassEncoderInsertDebugMarker(GPURenderPassEncoder* renderPassEncoder, const char* markerLabel)
{
    renderPassEncoder->InsertDebugMarker(markerLabel);
}

/* Buffer */
static GPUBufferDesc _GPUBufferDesc_Defaults(const GPUBufferDesc* desc) {
    GPUBufferDesc def = *desc;
    return def;
}

GPUBuffer* agpuCreateBuffer(GPUDevice* device, const GPUBufferDesc* desc, const void* pInitialData)
{
    if (!desc)
        return nullptr;

    GPUBufferDesc descDef = _GPUBufferDesc_Defaults(desc);

    // TODO: Validation
    //if (descDef.size > adapterProperties.limits.bufferMaxSize)
    //{
    //    alimerLogError("Buffer size too large: {}, limit: {}", desc.size, adapterProperties.limits.bufferMaxSize);
    //    return nullptr;
    //}

    return device->CreateBuffer(descDef, pInitialData);
}

void agpuBufferSetLabel(GPUBuffer* buffer, const char* label)
{
    buffer->SetLabel(label);
}

uint32_t agpuBufferAddRef(GPUBuffer* buffer)
{
    return buffer->AddRef();
}

uint32_t agpuBufferRelease(GPUBuffer* buffer)
{
    return buffer->Release();
}

uint64_t agpuBufferGetSize(GPUBuffer* buffer)
{
    return buffer->desc.size;
}

GPUDeviceAddress agpuBufferGetDeviceAddress(GPUBuffer* buffer)
{
    return buffer->GetDeviceAddress();
}

/* Texture */
static GPUTextureDesc _GPUTextureDesc_Defaults(const GPUTextureDesc* desc) {
    GPUTextureDesc def = *desc;
    def.dimension = _ALIMER_DEF(def.dimension, GPUTextureDimension_2D);
    def.format = _ALIMER_DEF(def.format, GPUPixelFormat_RGBA8Unorm);
    def.width = _ALIMER_DEF(def.width, 1u);
    def.height = _ALIMER_DEF(def.height, 1u);
    def.depthOrArrayLayers = _ALIMER_DEF(def.depthOrArrayLayers, 1u);
    if (def.mipLevelCount == 0)
    {
        def.mipLevelCount = GetMipLevelCount(def.width, def.height, def.depthOrArrayLayers);
    }
    def.sampleCount = _ALIMER_DEF(def.sampleCount, 1u);
    return def;
}

static bool ValidateTextureDesc(const GPUTextureDesc& desc)
{
    if (desc.width < 1 || desc.height < 1 || desc.depthOrArrayLayers < 1)
    {
        agpuLogError("Texture width, height and depthOrArrayLayers must be non-zero.");
        return false;
    }

    if (desc.format == GPUPixelFormat_Undefined)
    {
        agpuLogError("Texture format must be different than Undefined.");
        return false;
    }

    if ((desc.dimension == GPUTextureDimension_1D || desc.dimension == GPUTextureDimension_3D)
        && desc.sampleCount != 1)
    {
        agpuLogError("1D and 3D Textures must use TextureSampleCount.Count1.");
        return false;
    }

    return true;
}

GPUTexture* agpuCreateTexture(GPUDevice* device, const GPUTextureDesc* desc, const GPUTextureData* pInitialData)
{
    if (!desc)
        return nullptr;

    GPUTextureDesc descDef = _GPUTextureDesc_Defaults(desc);
    if (!ValidateTextureDesc(descDef))
    {
        return nullptr;
    }

    return device->CreateTexture(descDef, pInitialData);
}

void agpuTextureSetLabel(GPUTexture* texture, const char* label)
{
    texture->SetLabel(label);
}

GPUTextureDimension agpuTextureGetDimension(GPUTexture* texture)
{
    return texture->desc.dimension;
}

GPUPixelFormat agpuTextureGetFormat(GPUTexture* texture)
{
    return texture->desc.format;
}

GPUTextureUsage agpuTextureGetUsage(GPUTexture* texture)
{
    return texture->desc.usage;
}

uint32_t agpuTextureGetWidth(GPUTexture* texture)
{
    return texture->desc.width;
}

uint32_t agpuTextureGetHeight(GPUTexture* texture)
{
    return texture->desc.height;
}

uint32_t agpuTextureGetDepthOrArrayLayers(GPUTexture* texture)
{
    return texture->desc.depthOrArrayLayers;
}

uint32_t agpuTextureGetMipLevelCount(GPUTexture* texture)
{
    return texture->desc.mipLevelCount;
}

uint32_t agpuTextureGetSampleCount(GPUTexture* texture)
{
    return texture->desc.sampleCount;
}

uint32_t agpuTextureGetLevelWidth(GPUTexture* texture, uint32_t mipLevel)
{
    return std::max(texture->desc.width >> mipLevel, 1u);
}

uint32_t agpuTextureGetLevelHeight(GPUTexture* texture, uint32_t mipLevel)
{
    return std::max(texture->desc.height >> mipLevel, 1u);
}

uint32_t agpuTextureAddRef(GPUTexture* texture)
{
    return texture->AddRef();
}

uint32_t agpuTextureRelease(GPUTexture* texture)
{
    return texture->Release();
}

/* Sampler */
static GPUSamplerDesc _GPUSamplerDesc_Defaults(const GPUSamplerDesc* desc) {
    GPUSamplerDesc def = {};
    if (desc)
        def = *desc;

    return def;
}


GPUSampler* agpuCreateSampler(GPUDevice* device, const GPUSamplerDesc* desc)
{
    GPUSamplerDesc descDef = _GPUSamplerDesc_Defaults(desc);
    return device->CreateSampler(descDef);
}

void agpuSamplerSetLabel(GPUSampler* sampler, const char* label)
{
    sampler->SetLabel(label);
}

uint32_t agpuSamplerAddRef(GPUSampler* sampler)
{
    return sampler->AddRef();
}

uint32_t agpuSamplerRelease(GPUSampler* sampler)
{
    return sampler->Release();
}

/* PipelineLayout */
static GPUPipelineLayoutDesc _GPUPipelineLayoutDesc_Defaults(const GPUPipelineLayoutDesc* desc) {
    GPUPipelineLayoutDesc def = *desc;
    return def;
}

GPUPipelineLayout agpuCreatePipelineLayout(GPUDevice* device, const GPUPipelineLayoutDesc* desc)
{
    if (!desc)
        return nullptr;

    GPUPipelineLayoutDesc descDef = _GPUPipelineLayoutDesc_Defaults(desc);
    return device->CreatePipelineLayout(descDef);
}

void agpuPipelineLayoutSetLabel(GPUPipelineLayout pipelineLayout, const char* label)
{
    pipelineLayout->SetLabel(label);
}

uint32_t agpuPipelineLayoutAddRef(GPUPipelineLayout pipelineLayout)
{
    return pipelineLayout->AddRef();
}

uint32_t agpuPipelineLayoutRelease(GPUPipelineLayout pipelineLayout)
{
    return pipelineLayout->Release();
}

/* ShaderModule */
GPUShaderModule agpuCreateShaderModule(GPUDevice* device, const GPUShaderModuleDesc* desc)
{
    if (!desc)
    {
        return nullptr;
    }

    if (desc->stage == GPUShaderStage_Undefined)
    {
        agpuLogError("Invalid shader module stage");
        return nullptr;
    }

    if (desc->byteCodeSize == 0 || desc->byteCode == nullptr)
    {
        agpuLogError("Invalid shader module byteCode");
        return nullptr;
    }

    if (!desc->entryPoint || strlen(desc->entryPoint) == 0)
    {
        agpuLogError("Invalid shader module entryPoint");
        return nullptr;
    }

    return device->CreateShaderModule(desc);
}

void agpuShaderModuleSetLabel(GPUShaderModule shaderModule, const char* label)
{
    shaderModule->SetLabel(label);
}

uint32_t agpuShaderModuleAddRef(GPUShaderModule shaderModule)
{
    return shaderModule->AddRef();
}

uint32_t agpuShaderModuleRelease(GPUShaderModule shaderModule)
{
    return shaderModule->Release();
}

/* ComputePipeline */
static GPUComputePipelineDesc _GPUComputePipelineDesc_Defaults(const GPUComputePipelineDesc* desc) {
    GPUComputePipelineDesc def = *desc;
    return def;
}

GPUComputePipeline agpuCreateComputePipeline(GPUDevice* device, const GPUComputePipelineDesc* desc)
{
    if (!desc)
        return nullptr;

    if (!desc->shader)
    {
        agpuLogError("CreateComputePipeline: Invalid shader module");
        return nullptr;
    }

    // ArgumentException.ThrowIfFalse(descriptor.ComputeShader.Stage == ShaderStages.Compute, nameof(ComputePipelineDescriptor.ComputeShader));

    GPUComputePipelineDesc descDef = _GPUComputePipelineDesc_Defaults(desc);
    return device->CreateComputePipeline(descDef);
}

void agpuComputePipelineSetLabel(GPUComputePipeline computePipeline, const char* label)
{
    computePipeline->SetLabel(label);
}

uint32_t agpuComputePipelineAddRef(GPUComputePipeline computePipeline)
{
    return computePipeline->AddRef();
}

uint32_t agpuComputePipelineRelease(GPUComputePipeline computePipeline)
{
    return computePipeline->Release();
}

/* RenderPipeline */
static GPURenderPipelineDesc _GPURenderPipelineDesc_Defaults(const GPURenderPipelineDesc* desc) {
    GPURenderPipelineDesc def = *desc;

    // RasterizerState
    def.rasterizerState.fillMode = _ALIMER_DEF(def.rasterizerState.fillMode, GPUFillMode_Solid);
    def.rasterizerState.cullMode = _ALIMER_DEF(def.rasterizerState.cullMode, GPUCullMode_Back);
    def.rasterizerState.frontFace = _ALIMER_DEF(def.rasterizerState.frontFace, GPUFrontFace_Clockwise);
    def.rasterizerState.depthClipMode = _ALIMER_DEF(def.rasterizerState.depthClipMode, GPUDepthClipMode_Clip);

    // DepthStencilState
    def.depthStencilState.depthCompareFunction = _ALIMER_DEF(def.depthStencilState.depthCompareFunction, GPUCompareFunction_Always);
    def.depthStencilState.stencilReadMask = _ALIMER_DEF(def.depthStencilState.stencilReadMask, 0xFF);
    def.depthStencilState.stencilWriteMask = _ALIMER_DEF(def.depthStencilState.stencilReadMask, 0xFF);
    def.depthStencilState.frontFace.compareFunction = _ALIMER_DEF(def.depthStencilState.frontFace.compareFunction, GPUCompareFunction_Always);
    def.depthStencilState.frontFace.failOperation = _ALIMER_DEF(def.depthStencilState.frontFace.failOperation, GPUStencilOperation_Keep);
    def.depthStencilState.frontFace.depthFailOperation = _ALIMER_DEF(def.depthStencilState.frontFace.depthFailOperation, GPUStencilOperation_Keep);
    def.depthStencilState.frontFace.passOperation = _ALIMER_DEF(def.depthStencilState.frontFace.passOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.compareFunction = _ALIMER_DEF(def.depthStencilState.backFace.compareFunction, GPUCompareFunction_Always);
    def.depthStencilState.backFace.failOperation = _ALIMER_DEF(def.depthStencilState.backFace.failOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.depthFailOperation = _ALIMER_DEF(def.depthStencilState.backFace.depthFailOperation, GPUStencilOperation_Keep);
    def.depthStencilState.backFace.passOperation = _ALIMER_DEF(def.depthStencilState.backFace.passOperation, GPUStencilOperation_Keep);

    def.primitiveTopology = _ALIMER_DEF(def.primitiveTopology, GPUPrimitiveTopology_TriangleList);
    def.multisample.count = _ALIMER_DEF(def.multisample.count, 1u);
    def.multisample.mask = _ALIMER_DEF(def.multisample.mask, UINT32_MAX);

    for (uint32_t i = 0; i < def.colorAttachmentCount; ++i)
    {
        if (def.colorAttachments[i].format == GPUPixelFormat_Undefined)
            break;

        GPURenderPipelineColorAttachmentDesc& attachment = def.colorAttachments[i];
        attachment.srcColorBlendFactor = _ALIMER_DEF(attachment.srcColorBlendFactor, GPUBlendFactor_One);
        attachment.destColorBlendFactor = _ALIMER_DEF(attachment.destColorBlendFactor, GPUBlendFactor_Zero);
        attachment.colorBlendOperation = _ALIMER_DEF(attachment.colorBlendOperation, GPUBlendOperation_Add);
        attachment.srcAlphaBlendFactor = _ALIMER_DEF(attachment.srcAlphaBlendFactor, GPUBlendFactor_One);
        attachment.destAlphaBlendFactor = _ALIMER_DEF(attachment.destAlphaBlendFactor, GPUBlendFactor_Zero);
        attachment.alphaBlendOperation = _ALIMER_DEF(attachment.alphaBlendOperation, GPUBlendOperation_Add);
    }

    return def;
}

GPURenderPipeline agpuCreateRenderPipeline(GPUDevice* device, const GPURenderPipelineDesc* desc)
{
    if (!desc)
        return nullptr;

    GPURenderPipelineDesc descDef = _GPURenderPipelineDesc_Defaults(desc);
    return device->CreateRenderPipeline(descDef);
}

void agpuRenderPipelineSetLabel(GPURenderPipeline renderPipeline, const char* label)
{
    renderPipeline->SetLabel(label);
}

uint32_t agpuRenderPipelineAddRef(GPURenderPipeline renderPipeline)
{
    return renderPipeline->AddRef();
}

uint32_t agpuRenderPipelineRelease(GPURenderPipeline renderPipeline)
{
    return renderPipeline->Release();
}

/* QueryHeap */
static GPUQueryHeapDesc _GPUQueryHeapDesc_Defaults(const GPUQueryHeapDesc* desc)
{
    GPUQueryHeapDesc def = *desc;
    def.queryType = _ALIMER_DEF(def.queryType, GPUQueryType_Timestamp);
    def.count = _ALIMER_DEF(def.count, 1u);
    return def;
}

GPUQueryHeap* agpuCreateQueryHeap(GPUDevice* device, const GPUQueryHeapDesc* desc)
{
    if (!desc)
        return nullptr;

    GPUQueryHeapDesc descDef = _GPUQueryHeapDesc_Defaults(desc);
    return device->CreateQueryHeap(descDef);
}

void agpuQueryHeapSetLabel(GPUQueryHeap* queryHeap, const char* label)
{
    queryHeap->SetLabel(label);
}

uint32_t agpuQueryHeapAddRef(GPUQueryHeap* queryHeap)
{
    return queryHeap->AddRef();
}

uint32_t agpuQueryHeapRelease(GPUQueryHeap* queryHeap)
{
    return queryHeap->Release();
}

/* Other */
struct VertexFormatInfo
{
    GPUVertexFormat format;
    uint32_t byteSize;
    uint32_t componentCount;
};

static const VertexFormatInfo kVertexFormatTable[] = {
    { GPUVertexFormat_Undefined,           0, 0 },
    { GPUVertexFormat_UByte,               1, 1 },
    { GPUVertexFormat_UByte2,              2, 2 },
    { GPUVertexFormat_UByte4,              4, 4 },
    { GPUVertexFormat_Byte,                1, 1 },
    { GPUVertexFormat_Byte2,               2, 2 },
    { GPUVertexFormat_Byte4,               4, 4 },
    { GPUVertexFormat_UByteNormalized,     1, 1 },
    { GPUVertexFormat_UByte2Normalized,    2, 2 },
    { GPUVertexFormat_UByte4Normalized,    4, 4 },
    { GPUVertexFormat_ByteNormalized,      2, 2 },
    { GPUVertexFormat_Byte2Normalized,     2, 2 },
    { GPUVertexFormat_Byte4Normalized,     4, 4 },

    { GPUVertexFormat_UShort,              2, 1 },
    { GPUVertexFormat_UShort2,             4, 2 },
    { GPUVertexFormat_UShort4,             8, 4 },
    { GPUVertexFormat_Short,               4, 1 },
    { GPUVertexFormat_Short2,              4, 2 },
    { GPUVertexFormat_Short4,              8, 4 },
    { GPUVertexFormat_UShortNormalized,    2, 1 },
    { GPUVertexFormat_UShort2Normalized,   4, 2 },
    { GPUVertexFormat_UShort4Normalized,   8, 4 },
    { GPUVertexFormat_ShortNormalized,     2, 1 },
    { GPUVertexFormat_Short2Normalized,    4, 2 },
    { GPUVertexFormat_Short4Normalized,    8, 4 },

    { GPUVertexFormat_Half,                2, 1 },
    { GPUVertexFormat_Half2,               4, 2 },
    { GPUVertexFormat_Half4,               8, 4 },
    { GPUVertexFormat_Float,               4, 1 },
    { GPUVertexFormat_Float2,              8, 2 },
    { GPUVertexFormat_Float3,              12, 3 },
    { GPUVertexFormat_Float4,              16, 4 },

    { GPUVertexFormat_UInt,                4, 1 },
    { GPUVertexFormat_UInt2,               8, 2 },
    { GPUVertexFormat_UInt3,               12, 3 },
    { GPUVertexFormat_UInt4,               16, 4 },

    { GPUVertexFormat_Int,                 4, 1 },
    { GPUVertexFormat_Int2,                8, 2 },
    { GPUVertexFormat_Int3,                12, 3 },
    { GPUVertexFormat_Int4,                16, 4 },

    { GPUVertexFormat_Unorm10_10_10_2, 4, 4 },
    { GPUVertexFormat_Unorm8x4BGRA, 4, 4 }
    //{VertexFormat::RG11B10Float,   32, 4,  VertexFormatKind::Float},
    //{VertexFormat::RGB9E5Float,   32, 4, VertexFormatKind::Float},
};

static_assert(
    sizeof(kVertexFormatTable) / sizeof(VertexFormatInfo) == size_t(_GPUVertexFormat_Count),
    "The format info table doesn't have the right number of elements"
    );

enum class KnownGPUAdapterVendor
{
    AMD = 0x01002,
    NVIDIA = 0x010DE,
    INTEL = 0x08086,
    ARM = 0x013B5,
    QUALCOMM = 0x05143,
    IMGTECH = 0x01010,
    MSFT = 0x01414,
    APPLE = 0x0106B,
    MESA = 0x10005,
    BROADCOM = 0x014e4
};

static const VertexFormatInfo& GetVertexFormatInfo(GPUVertexFormat format)
{
    if (format >= _GPUVertexFormat_Count)
        return kVertexFormatTable[0]; // Undefined

    const VertexFormatInfo& info = kVertexFormatTable[format];
    ALIMER_ASSERT(info.format == format);
    return info;
}

uint32_t agpuGetVertexFormatByteSize(GPUVertexFormat format)
{
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.byteSize;
}

uint32_t agpuGetVertexFormatComponentCount(GPUVertexFormat format)
{
    const VertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.componentCount;
}

GPUAdapterVendor agpuGPUAdapterVendorFromID(uint32_t vendorId)
{
    switch (vendorId)
    {
        case (uint32_t)KnownGPUAdapterVendor::AMD:
            return GPUAdapterVendor_AMD;
        case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
            return GPUAdapterVendor_NVIDIA;
        case (uint32_t)KnownGPUAdapterVendor::INTEL:
            return GPUAdapterVendor_Intel;
        case (uint32_t)KnownGPUAdapterVendor::ARM:
            return GPUAdapterVendor_ARM;
        case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
            return GPUAdapterVendor_Qualcomm;
        case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
            return GPUAdapterVendor_ImgTech;
        case (uint32_t)KnownGPUAdapterVendor::MSFT:
            return GPUAdapterVendor_MSFT;
        case (uint32_t)KnownGPUAdapterVendor::APPLE:
            return GPUAdapterVendor_Apple;
        case (uint32_t)KnownGPUAdapterVendor::MESA:
            return GPUAdapterVendor_Mesa;
        case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
            return GPUAdapterVendor_Broadcom;

        default:
            return GPUAdapterVendor_Unknown;
    }
}

uint32_t agpuGPUAdapterVendorToID(GPUAdapterVendor vendor)
{
    switch (vendor)
    {
        case GPUAdapterVendor_AMD:
            return (uint32_t)KnownGPUAdapterVendor::AMD;
        case GPUAdapterVendor_NVIDIA:
            return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
        case GPUAdapterVendor_Intel:
            return (uint32_t)KnownGPUAdapterVendor::INTEL;
        case GPUAdapterVendor_ARM:
            return (uint32_t)KnownGPUAdapterVendor::ARM;
        case GPUAdapterVendor_Qualcomm:
            return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
        case GPUAdapterVendor_ImgTech:
            return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
        case GPUAdapterVendor_MSFT:
            return (uint32_t)KnownGPUAdapterVendor::MSFT;
        case GPUAdapterVendor_Apple:
            return (uint32_t)KnownGPUAdapterVendor::APPLE;
        case GPUAdapterVendor_Mesa:
            return (uint32_t)KnownGPUAdapterVendor::MESA;
        case GPUAdapterVendor_Broadcom:
            return (uint32_t)KnownGPUAdapterVendor::BROADCOM;

        default:
            return 0;
    }
}
