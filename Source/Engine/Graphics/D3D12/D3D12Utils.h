// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Log.h"
#include "Graphics/GraphicsDefs.h"
#include "PlatformInclude.h"

#include <wrl/client.h>

#include <dxgi1_6.h>

#include "directx/d3d12.h"
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemAlloc.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace Alimer
{
	class D3D12Graphics;
	class D3D12Texture;
	class D3D12CommandBuffer;
	class D3D12SwapChain;
	class D3D12Pipeline;
	class D3D12CommandBuffer;
	class D3D12CommandQueue;

	struct D3D12DescriptorAlloc
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = { };
		uint32_t index = uint32_t(-1);
	};

	constexpr const char* ToString(D3D_FEATURE_LEVEL value)
	{
		switch (value)
		{
		case D3D_FEATURE_LEVEL_11_0:
			return "Level 11.0";
		case D3D_FEATURE_LEVEL_11_1:
			return "Level 11.1";
		case D3D_FEATURE_LEVEL_12_0:
			return "Level 12.0";
		case D3D_FEATURE_LEVEL_12_1:
			return "Level 12.1";
		case D3D_FEATURE_LEVEL_12_2:
			return "Level 12.2";
		default:
			return nullptr;
		}
	}

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
            ALIMER_UNREACHABLE();
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    [[nodiscard]] constexpr PixelFormat FromDXGIFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
            // 8-bit formats
        case DXGI_FORMAT_R8_UNORM:	return PixelFormat::R8Unorm;
        case DXGI_FORMAT_R8_SNORM:	return PixelFormat::R8Snorm;
        case DXGI_FORMAT_R8_UINT:	return PixelFormat::R8Uint;
        case DXGI_FORMAT_R8_SINT:	return PixelFormat::R8Sint;
            // 16-bit formats
        case DXGI_FORMAT_R16_UNORM:		return PixelFormat::R16Unorm;
        case DXGI_FORMAT_R16_SNORM:		return PixelFormat::R16Snorm;
        case DXGI_FORMAT_R16_UINT:		return PixelFormat::R16Uint;
        case DXGI_FORMAT_R16_SINT:		return PixelFormat::R16Sint;
        case DXGI_FORMAT_R16_FLOAT:		return PixelFormat::R16Float;
        case DXGI_FORMAT_R8G8_UNORM:	return PixelFormat::RG8Unorm;
        case DXGI_FORMAT_R8G8_SNORM:	return PixelFormat::RG8Snorm;
        case DXGI_FORMAT_R8G8_UINT:		return PixelFormat::RG8Uint;
        case DXGI_FORMAT_R8G8_SINT:		return PixelFormat::RG8Sint;
            // 32-bit formats
        case DXGI_FORMAT_R32_UINT:				return PixelFormat::R32Uint;
        case DXGI_FORMAT_R32_SINT:				return PixelFormat::R32Sint;
        case DXGI_FORMAT_R32_FLOAT:				return PixelFormat::R32Float;
        case DXGI_FORMAT_R16G16_UNORM:			return PixelFormat::RG16Unorm;
        case DXGI_FORMAT_R16G16_SNORM:			return PixelFormat::RG16Snorm;
        case DXGI_FORMAT_R16G16_UINT:			return PixelFormat::RG16Uint;
        case DXGI_FORMAT_R16G16_SINT:			return PixelFormat::RG16Sint;
        case DXGI_FORMAT_R16G16_FLOAT:			return PixelFormat::RG16Float;
        case DXGI_FORMAT_R8G8B8A8_UNORM:		return PixelFormat::RGBA8Unorm;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:	return PixelFormat::RGBA8UnormSrgb;
        case DXGI_FORMAT_R8G8B8A8_SNORM:		return PixelFormat::RGBA8Snorm;
        case DXGI_FORMAT_R8G8B8A8_UINT:			return PixelFormat::RGBA8Uint;
        case DXGI_FORMAT_R8G8B8A8_SINT:			return PixelFormat::RGBA8Sint;
        case DXGI_FORMAT_B8G8R8A8_UNORM:		return PixelFormat::BGRA8Unorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:	return PixelFormat::BGRA8UnormSrgb;
            // Packed 32-Bit formats
        case DXGI_FORMAT_R10G10B10A2_UNORM:		return PixelFormat::RGB10A2Unorm;
        case DXGI_FORMAT_R11G11B10_FLOAT:		return PixelFormat::RG11B10Float;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:    return PixelFormat::RGB9E5Float;
            // 64-Bit formats
        case DXGI_FORMAT_R32G32_UINT:			return PixelFormat::RG32Uint;
        case DXGI_FORMAT_R32G32_SINT:			return PixelFormat::RG32Sint;
        case DXGI_FORMAT_R32G32_FLOAT:			return PixelFormat::RG32Float;
        case DXGI_FORMAT_R16G16B16A16_UNORM:	return PixelFormat::RGBA16Unorm;
        case DXGI_FORMAT_R16G16B16A16_SNORM:	return PixelFormat::RGBA16Snorm;
        case DXGI_FORMAT_R16G16B16A16_UINT:		return PixelFormat::RGBA16Uint;
        case DXGI_FORMAT_R16G16B16A16_SINT:		return PixelFormat::RGBA16Sint;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:	return PixelFormat::RGBA16Float;
            // 128-Bit formats
        case DXGI_FORMAT_R32G32B32A32_UINT:		return PixelFormat::RGBA32Uint;
        case DXGI_FORMAT_R32G32B32A32_SINT:		return PixelFormat::RGBA32Sint;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:	return PixelFormat::RGBA32Float;
            // Depth-stencil formats
        case DXGI_FORMAT_D16_UNORM:				return PixelFormat::Depth16Unorm;
        case DXGI_FORMAT_D32_FLOAT:				return PixelFormat::Depth32Float;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:		return PixelFormat::Depth24UnormStencil8;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:	return PixelFormat::Depth32FloatStencil8;
            // Compressed BC formats
        case DXGI_FORMAT_BC1_UNORM:			return PixelFormat::BC1RGBAUnorm;
        case DXGI_FORMAT_BC1_UNORM_SRGB:	return PixelFormat::BC1RGBAUnormSrgb;
        case DXGI_FORMAT_BC2_UNORM:			return PixelFormat::BC2RGBAUnorm;
        case DXGI_FORMAT_BC2_UNORM_SRGB:	return PixelFormat::BC2RGBAUnormSrgb;
        case DXGI_FORMAT_BC3_UNORM:			return PixelFormat::BC3RGBAUnorm;
        case DXGI_FORMAT_BC3_UNORM_SRGB:	return PixelFormat::BC3RGBAUnormSrgb;
        case DXGI_FORMAT_BC4_SNORM:			return PixelFormat::BC4RSnorm;
        case DXGI_FORMAT_BC4_UNORM:			return PixelFormat::BC4RUnorm;
        case DXGI_FORMAT_BC5_SNORM:			return PixelFormat::BC5RGSnorm;
        case DXGI_FORMAT_BC5_UNORM:			return PixelFormat::BC5RGUnorm;
        case DXGI_FORMAT_BC6H_SF16:			return PixelFormat::BC6HRGBFloat;
        case DXGI_FORMAT_BC6H_UF16:			return PixelFormat::BC6HRGBUFloat;
        case DXGI_FORMAT_BC7_UNORM:			return PixelFormat::BC7RGBAUnorm;
        case DXGI_FORMAT_BC7_UNORM_SRGB:	return PixelFormat::BC7RGBAUnormSrgb;

        default:
            ALIMER_UNREACHABLE();
            return PixelFormat::Undefined;
        }
    }

	[[nodiscard]] constexpr D3D12_COMMAND_LIST_TYPE ToD3D12(CommandQueueType type)
	{
		switch (type)
		{
		case CommandQueueType::Graphics:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case CommandQueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		//case CommandQueueType::Copy:
		//	return D3D12_COMMAND_LIST_TYPE_COPY;
		default:
			ALIMER_UNREACHABLE();
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	[[nodiscard]] constexpr D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(CompareFunction function)
	{
		switch (function)
		{
		case CompareFunction::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case CompareFunction::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case CompareFunction::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case CompareFunction::LessEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case CompareFunction::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case CompareFunction::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case CompareFunction::GreaterEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case CompareFunction::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;

		default:
			ALIMER_UNREACHABLE();
			return static_cast<D3D12_COMPARISON_FUNC>(0);
		}
	}

    [[nodiscard]] constexpr uint32_t D3D12SampleCount(SampleCount count)
    {
        switch (count)
        {
            case SampleCount::Count1:
                return 1;
            case SampleCount::Count2:
                return 2;
            case SampleCount::Count4:
                return 4;
            case SampleCount::Count8:
                return 8;
            case SampleCount::Count16:
                return 16;
            case SampleCount::Count32:
                return 32;

            default:
                ALIMER_UNREACHABLE();
                return 1;
        }
    }

	[[nodiscard]] constexpr DXGI_FORMAT ToDXGISwapChainFormat(PixelFormat format)
	{
		switch (format) {
		case PixelFormat::RGBA16Float:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case PixelFormat::BGRA8Unorm:
		case PixelFormat::BGRA8UnormSrgb:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		case PixelFormat::RGBA8Unorm:
		case PixelFormat::RGBA8UnormSrgb:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case PixelFormat::RGB10A2Unorm:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		}

		return DXGI_FORMAT_B8G8R8A8_UNORM;
	}

	class D3D12GpuResource
	{
		friend class D3D12CommandBuffer;

	public:
		D3D12GpuResource() = default;
		D3D12GpuResource(ID3D12Resource* resource_, D3D12_RESOURCE_STATES currentState) 
			: handle(resource_)
			, state(currentState)
			, transitioningState((D3D12_RESOURCE_STATES)-1)
		{
		}

		virtual ~D3D12GpuResource()
		{
			Destroy();
		}

		virtual void Destroy()
		{
            handle = nullptr;
			allocation = nullptr;
			gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		}

		ID3D12Resource* GetHandle() const { return handle; }

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return gpuVirtualAddress; }

	protected:
		ID3D12Resource* handle = nullptr;
		D3D12MA::Allocation* allocation = nullptr;
		mutable D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
		mutable D3D12_RESOURCE_STATES transitioningState = (D3D12_RESOURCE_STATES)-1;
		D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		bool fixedResourceState = false;
	};
}
