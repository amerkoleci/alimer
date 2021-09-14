// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "PlatformInclude.h"
#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Alimer
{
    // Type alias for ComPtr template
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    template <typename T>
    void SafeRelease(T& resource)
    {
        if (resource)
        {
            resource->Release();
            resource = nullptr;
        }
    }

    [[nodiscard]] constexpr DXGI_FORMAT ToDXGIFormat(PixelFormat format)
    {
        switch (format)
        {
            // 8-bit formats
        case PixelFormat::R8Unorm:  return DXGI_FORMAT_R8_UNORM;
        case PixelFormat::R8Snorm:  return DXGI_FORMAT_R8_SNORM;
        case PixelFormat::R8Uint:   return DXGI_FORMAT_R8_UINT;
        case PixelFormat::R8Sint:   return DXGI_FORMAT_R8_SINT;
            // 16-bit formats
        case PixelFormat::R16Unorm:     return DXGI_FORMAT_R16_UNORM;
        case PixelFormat::R16Snorm:     return DXGI_FORMAT_R16_SNORM;
        case PixelFormat::R16Uint:      return DXGI_FORMAT_R16_UINT;
        case PixelFormat::R16Sint:      return DXGI_FORMAT_R16_SINT;
        case PixelFormat::R16Float:     return DXGI_FORMAT_R16_FLOAT;
        case PixelFormat::RG8Unorm:     return DXGI_FORMAT_R8G8_UNORM;
        case PixelFormat::RG8Snorm:     return DXGI_FORMAT_R8G8_SNORM;
        case PixelFormat::RG8Uint:      return DXGI_FORMAT_R8G8_UINT;
        case PixelFormat::RG8Sint:      return DXGI_FORMAT_R8G8_SINT;
            // Packed 16-Bit Pixel Formats
        case PixelFormat::BGRA4Unorm:       return DXGI_FORMAT_B4G4R4A4_UNORM;
        case PixelFormat::B5G6R5Unorm:      return DXGI_FORMAT_B5G6R5_UNORM;
        case PixelFormat::B5G5R5A1Unorm:    return DXGI_FORMAT_B5G5R5A1_UNORM;
            // 32-bit formats
        case PixelFormat::R32Uint:          return DXGI_FORMAT_R32_UINT;
        case PixelFormat::R32Sint:          return DXGI_FORMAT_R32_SINT;
        case PixelFormat::R32Float:         return DXGI_FORMAT_R32_FLOAT;
        case PixelFormat::RG16Unorm:        return DXGI_FORMAT_R16G16_UNORM;
        case PixelFormat::RG16Snorm:        return DXGI_FORMAT_R16G16_SNORM;
        case PixelFormat::RG16Uint:         return DXGI_FORMAT_R16G16_UINT;
        case PixelFormat::RG16Sint:         return DXGI_FORMAT_R16G16_SINT;
        case PixelFormat::RG16Float:        return DXGI_FORMAT_R16G16_FLOAT;
        case PixelFormat::RGBA8Unorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::RGBA8UnormSrgb:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case PixelFormat::RGBA8Snorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
        case PixelFormat::RGBA8Uint:        return DXGI_FORMAT_R8G8B8A8_UINT;
        case PixelFormat::RGBA8Sint:        return DXGI_FORMAT_R8G8B8A8_SINT;
        case PixelFormat::BGRA8Unorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::BGRA8UnormSrgb:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            // Packed 32-Bit formats
        case PixelFormat::RGB10A2Unorm:     return DXGI_FORMAT_R10G10B10A2_UNORM;
        case PixelFormat::RG11B10Float:     return DXGI_FORMAT_R11G11B10_FLOAT;
        case PixelFormat::RGB9E5Float:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            // 64-Bit formats
        case PixelFormat::RG32Uint:         return DXGI_FORMAT_R32G32_UINT;
        case PixelFormat::RG32Sint:         return DXGI_FORMAT_R32G32_SINT;
        case PixelFormat::RG32Float:        return DXGI_FORMAT_R32G32_FLOAT;
        case PixelFormat::RGBA16Unorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
        case PixelFormat::RGBA16Snorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
        case PixelFormat::RGBA16Uint:       return DXGI_FORMAT_R16G16B16A16_UINT;
        case PixelFormat::RGBA16Sint:       return DXGI_FORMAT_R16G16B16A16_SINT;
        case PixelFormat::RGBA16Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
            // 128-Bit formats
        case PixelFormat::RGBA32Uint:       return DXGI_FORMAT_R32G32B32A32_UINT;
        case PixelFormat::RGBA32Sint:       return DXGI_FORMAT_R32G32B32A32_SINT;
        case PixelFormat::RGBA32Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
            // Depth-stencil formats
        case PixelFormat::Depth16Unorm:			return DXGI_FORMAT_D16_UNORM;
        case PixelFormat::Depth32Float:			return DXGI_FORMAT_D32_FLOAT;
        case PixelFormat::Depth24UnormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            // Compressed BC formats
        case PixelFormat::BC1RGBAUnorm:         return DXGI_FORMAT_BC1_UNORM;
        case PixelFormat::BC1RGBAUnormSrgb:     return DXGI_FORMAT_BC1_UNORM_SRGB;
        case PixelFormat::BC2RGBAUnorm:         return DXGI_FORMAT_BC2_UNORM;
        case PixelFormat::BC2RGBAUnormSrgb:     return DXGI_FORMAT_BC2_UNORM_SRGB;
        case PixelFormat::BC3RGBAUnorm:         return DXGI_FORMAT_BC3_UNORM;
        case PixelFormat::BC3RGBAUnormSrgb:     return DXGI_FORMAT_BC3_UNORM_SRGB;
        case PixelFormat::BC4RSnorm:            return DXGI_FORMAT_BC4_SNORM;
        case PixelFormat::BC4RUnorm:            return DXGI_FORMAT_BC4_UNORM;
        case PixelFormat::BC5RGSnorm:           return DXGI_FORMAT_BC5_SNORM;
        case PixelFormat::BC5RGUnorm:           return DXGI_FORMAT_BC5_UNORM;
        case PixelFormat::BC6HRGBFloat:         return DXGI_FORMAT_BC6H_SF16;
        case PixelFormat::BC6HRGBUFloat:        return DXGI_FORMAT_BC6H_UF16;
        case PixelFormat::BC7RGBAUnorm:         return DXGI_FORMAT_BC7_UNORM;
        case PixelFormat::BC7RGBAUnormSrgb:     return DXGI_FORMAT_BC7_UNORM_SRGB;

        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    [[nodiscard]] constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::Depth16Unorm:
            return DXGI_FORMAT_R16_TYPELESS;
        case PixelFormat::Depth32Float:
            return DXGI_FORMAT_R32_TYPELESS;
        case PixelFormat::Depth24UnormStencil8:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case PixelFormat::Depth32FloatStencil8:
            return DXGI_FORMAT_R32G8X24_TYPELESS;

        default:
            ALIMER_ASSERT(IsDepthFormat(format) == false);
            return ToDXGIFormat(format);
        }
    }
}

