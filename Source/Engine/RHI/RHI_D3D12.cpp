// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_D3D11)
#include "Window.h"
#include "Core/StringUtils.h"
#include "Core/Log.h"
#include "RHI_D3D12.h"

#include "directx/d3dx12.h"
//#include "directx/dxcapi.h"
#include "directx/d3d12shader.h"
#include <pix.h>

#if !defined(ALIMER_DISABLE_SHADER_COMPILER)
#include <d3dcompiler.h>
#endif

#include <array>

namespace alimer::rhi
{
    DXGI_FORMAT ToDXGIFormat(Format format)
    {
        switch (format)
        {
            // 8-bit formats
            case Format::R8UNorm:  return DXGI_FORMAT_R8_UNORM;
            case Format::R8SNorm:  return DXGI_FORMAT_R8_SNORM;
            case Format::R8UInt:   return DXGI_FORMAT_R8_UINT;
            case Format::R8SInt:   return DXGI_FORMAT_R8_SINT;
                // 16-bit formats
            case Format::R16UNorm:     return DXGI_FORMAT_R16_UNORM;
            case Format::R16SNorm:     return DXGI_FORMAT_R16_SNORM;
            case Format::R16UInt:      return DXGI_FORMAT_R16_UINT;
            case Format::R16SInt:      return DXGI_FORMAT_R16_SINT;
            case Format::R16Float:     return DXGI_FORMAT_R16_FLOAT;
            case Format::RG8UNorm:     return DXGI_FORMAT_R8G8_UNORM;
            case Format::RG8SNorm:     return DXGI_FORMAT_R8G8_SNORM;
            case Format::RG8UInt:      return DXGI_FORMAT_R8G8_UINT;
            case Format::RG8SInt:      return DXGI_FORMAT_R8G8_SINT;
                // Packed 16-Bit Pixel Formats
            case Format::BGRA4UNorm:       return DXGI_FORMAT_B4G4R4A4_UNORM;
            case Format::B5G6R5UNorm:      return DXGI_FORMAT_B5G6R5_UNORM;
            case Format::B5G5R5A1UNorm:    return DXGI_FORMAT_B5G5R5A1_UNORM;
                // 32-bit formats
            case Format::R32UInt:          return DXGI_FORMAT_R32_UINT;
            case Format::R32SInt:          return DXGI_FORMAT_R32_SINT;
            case Format::R32Float:         return DXGI_FORMAT_R32_FLOAT;
            case Format::RG16UNorm:        return DXGI_FORMAT_R16G16_UNORM;
            case Format::RG16SNorm:        return DXGI_FORMAT_R16G16_SNORM;
            case Format::RG16UInt:         return DXGI_FORMAT_R16G16_UINT;
            case Format::RG16SInt:         return DXGI_FORMAT_R16G16_SINT;
            case Format::RG16Float:        return DXGI_FORMAT_R16G16_FLOAT;
            case Format::RGBA8UNorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Format::RGBA8UNormSrgb:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case Format::RGBA8SNorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
            case Format::RGBA8UInt:        return DXGI_FORMAT_R8G8B8A8_UINT;
            case Format::RGBA8SInt:        return DXGI_FORMAT_R8G8B8A8_SINT;
            case Format::BGRA8UNorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::BGRA8UNormSrgb:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                // Packed 32-Bit formats
            case Format::RGB10A2UNorm:     return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Format::RG11B10Float:     return DXGI_FORMAT_R11G11B10_FLOAT;
            case Format::RGB9E5Float:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                // 64-Bit formats
            case Format::RG32UInt:         return DXGI_FORMAT_R32G32_UINT;
            case Format::RG32SInt:         return DXGI_FORMAT_R32G32_SINT;
            case Format::RG32Float:        return DXGI_FORMAT_R32G32_FLOAT;
            case Format::RGBA16UNorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
            case Format::RGBA16SNorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
            case Format::RGBA16UInt:       return DXGI_FORMAT_R16G16B16A16_UINT;
            case Format::RGBA16SInt:       return DXGI_FORMAT_R16G16B16A16_SINT;
            case Format::RGBA16Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
                // 128-Bit formats
            case Format::RGBA32UInt:       return DXGI_FORMAT_R32G32B32A32_UINT;
            case Format::RGBA32SInt:       return DXGI_FORMAT_R32G32B32A32_SINT;
            case Format::RGBA32Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
                // Depth-stencil formats
            case Format::Depth16UNorm:			return DXGI_FORMAT_D16_UNORM;
            case Format::Depth32Float:			return DXGI_FORMAT_D32_FLOAT;
            case Format::Depth24UNormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Format::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                // Compressed BC formats
            case Format::BC1UNorm:         return DXGI_FORMAT_BC1_UNORM;
            case Format::BC1UNormSrgb:     return DXGI_FORMAT_BC1_UNORM_SRGB;
            case Format::BC2UNorm:         return DXGI_FORMAT_BC2_UNORM;
            case Format::BC2UNormSrgb:     return DXGI_FORMAT_BC2_UNORM_SRGB;
            case Format::BC3UNorm:         return DXGI_FORMAT_BC3_UNORM;
            case Format::BC3UNormSrgb:     return DXGI_FORMAT_BC3_UNORM_SRGB;
            case Format::BC4SNorm:         return DXGI_FORMAT_BC4_SNORM;
            case Format::BC4UNorm:         return DXGI_FORMAT_BC4_UNORM;
            case Format::BC5SNorm:         return DXGI_FORMAT_BC5_SNORM;
            case Format::BC5UNorm:         return DXGI_FORMAT_BC5_UNORM;
            case Format::BC6HUFloat:       return DXGI_FORMAT_BC6H_UF16;
            case Format::BC6HSFloat:       return DXGI_FORMAT_BC6H_SF16;
            case Format::BC7UNorm:         return DXGI_FORMAT_BC7_UNORM;
            case Format::BC7UNormSrgb:     return DXGI_FORMAT_BC7_UNORM_SRGB;

            default:
                ALIMER_UNREACHABLE();
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    namespace
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
        static PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

        using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);
        static PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;

        static PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
        static PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
        static PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

        //static DxcCreateInstanceProc DxcCreateInstance;
#endif

#ifdef _DEBUG
        // Declare debug guids to avoid linking with "dxguid.lib"
        static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
        static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };
#endif

#if !defined(ALIMER_DISABLE_SHADER_COMPILER) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        pD3DCompile D3DCompile;
#endif

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

        inline void SetDebugName(ID3D12Resource* resource, const std::string_view& name)
        {
            auto wideName = ToUtf16(name);
            ThrowIfFailed(resource->SetName(wideName.c_str()));
        }

        [[nodiscard]] constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(Format format)
        {
            switch (format)
            {
                case Format::Depth16UNorm:
                    return DXGI_FORMAT_R16_TYPELESS;
                case Format::Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;
                case Format::Depth24UNormStencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case Format::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    ALIMER_ASSERT(IsDepthFormat(format) == false);
                    return ToDXGIFormat(format);
            }
        }

        [[nodiscard]] constexpr D3D_PRIMITIVE_TOPOLOGY ConvertPrimitiveTopology(PrimitiveTopology topology, uint32_t controlPoints)
        {
            switch (topology)
            {
                case PrimitiveTopology::PointList:
                    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case PrimitiveTopology::LineList:
                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case PrimitiveTopology::LineStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case PrimitiveTopology::TriangleList:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case PrimitiveTopology::TriangleStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                case PrimitiveTopology::PatchList:
                    if (controlPoints == 0 || controlPoints > 32)
                    {
                        assert(false && "Invalid PatchList control points");
                        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                    }
                    return D3D_PRIMITIVE_TOPOLOGY(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + (controlPoints - 1));
                default:
                    ALIMER_UNREACHABLE();
                    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        inline D3D12_RESOURCE_STATES ToD3D12(ResourceStates stateBits)
        {
            if (stateBits == ResourceStates::Unknown)
                return D3D12_RESOURCE_STATE_COMMON;

            if (stateBits == ResourceStates::Present)
                return D3D12_RESOURCE_STATE_PRESENT;

            D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
            if ((stateBits & ResourceStates::ConstantBuffer) != 0) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            if ((stateBits & ResourceStates::VertexBuffer) != 0) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            if ((stateBits & ResourceStates::IndexBuffer) != 0) result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
            if ((stateBits & ResourceStates::IndirectArgument) != 0) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            if ((stateBits & ResourceStates::ShaderResource) != 0) result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if ((stateBits & ResourceStates::UnorderedAccess) != 0) result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            if ((stateBits & ResourceStates::RenderTarget) != 0) result |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            if ((stateBits & ResourceStates::DepthWrite) != 0) result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
            if ((stateBits & ResourceStates::DepthRead) != 0) result |= D3D12_RESOURCE_STATE_DEPTH_READ;
            if ((stateBits & ResourceStates::StreamOut) != 0) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
            if ((stateBits & ResourceStates::CopyDest) != 0) result |= D3D12_RESOURCE_STATE_COPY_DEST;
            if ((stateBits & ResourceStates::CopySource) != 0) result |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            if ((stateBits & ResourceStates::ResolveDest) != 0) result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
            if ((stateBits & ResourceStates::ResolveSource) != 0) result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
            if ((stateBits & ResourceStates::Present) != 0) result |= D3D12_RESOURCE_STATE_PRESENT;
            if ((stateBits & ResourceStates::AccelerationStructureRead) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::AccelerationStructureBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if ((stateBits & ResourceStates::AccelerationStructureBuildBlas) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & ResourceStates::ShadingRateSurface) != 0) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

            return result;
        }

        [[nodiscard]] constexpr D3D12_COMPARISON_FUNC ToD3D12(CompareFunction function)
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

        [[nodiscard]] constexpr D3D12_BLEND D3D12Blend(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::Zero:
                    return D3D12_BLEND_ZERO;
                case BlendFactor::One:
                    return D3D12_BLEND_ONE;
                case BlendFactor::SourceColor:
                    return D3D12_BLEND_SRC_COLOR;
                case BlendFactor::OneMinusSourceColor:
                    return D3D12_BLEND_INV_SRC_COLOR;
                case BlendFactor::SourceAlpha:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::OneMinusSourceAlpha:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D12_BLEND_DEST_COLOR;
                case BlendFactor::OneMinusDestinationColor:
                    return D3D12_BLEND_INV_DEST_COLOR;
                case BlendFactor::DestinationAlpha:
                    return D3D12_BLEND_DEST_ALPHA;
                case BlendFactor::OneMinusDestinationAlpha:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case BlendFactor::SourceAlphaSaturated:
                    return D3D12_BLEND_SRC_ALPHA_SAT;
                case BlendFactor::BlendColor:
                    return D3D12_BLEND_BLEND_FACTOR;
                case BlendFactor::OneMinusBlendColor:
                    return D3D12_BLEND_INV_BLEND_FACTOR;
                case BlendFactor::Source1Color:
                    return D3D12_BLEND_SRC1_COLOR;
                case BlendFactor::OneMinusSource1Color:
                    return D3D12_BLEND_INV_SRC1_COLOR;
                case BlendFactor::Source1Alpha:
                    return D3D12_BLEND_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Alpha:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr D3D12_BLEND D3D12AlphaBlend(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::SourceColor:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::OneMinusSourceColor:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D12_BLEND_DEST_ALPHA;
                case BlendFactor::OneMinusDestinationColor:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case BlendFactor::SourceAlpha:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::Source1Color:
                    return D3D12_BLEND_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Color:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                    // Other blend factors translate to the same D3D12 enum as the color blend factors.
                default:
                    return D3D12Blend(factor);
            }
        }

        [[nodiscard]] constexpr D3D12_BLEND_OP D3D12BlendOperation(BlendOperation operation)
        {
            switch (operation)
            {
                case BlendOperation::Add:
                    return D3D12_BLEND_OP_ADD;
                case BlendOperation::Subtract:
                    return D3D12_BLEND_OP_SUBTRACT;
                case BlendOperation::ReverseSubtract:
                    return D3D12_BLEND_OP_REV_SUBTRACT;
                case BlendOperation::Min:
                    return D3D12_BLEND_OP_MIN;
                case BlendOperation::Max:
                    return D3D12_BLEND_OP_MAX;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr uint8_t D3D12RenderTargetWriteMask(ColorWriteMask mask)
        {
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Red) == D3D12_COLOR_WRITE_ENABLE_RED, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Green) == D3D12_COLOR_WRITE_ENABLE_GREEN, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Blue) == D3D12_COLOR_WRITE_ENABLE_BLUE, "ColorWriteMask mismatch");
            static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Alpha) == D3D12_COLOR_WRITE_ENABLE_ALPHA, "ColorWriteMask mismatch");
            return static_cast<uint8_t>(mask);
        }

        [[nodiscard]] constexpr D3D12_STENCIL_OP ToD3D12(StencilOperation op)
        {
            switch (op) {
                case StencilOperation::Keep:
                    return D3D12_STENCIL_OP_KEEP;
                case StencilOperation::Zero:
                    return D3D12_STENCIL_OP_ZERO;
                case StencilOperation::Replace:
                    return D3D12_STENCIL_OP_REPLACE;
                case StencilOperation::IncrementClamp:
                    return D3D12_STENCIL_OP_INCR_SAT;
                case StencilOperation::DecrementClamp:
                    return D3D12_STENCIL_OP_DECR_SAT;
                case StencilOperation::Invert:
                    return D3D12_STENCIL_OP_INVERT;
                case StencilOperation::IncrementWrap:
                    return D3D12_STENCIL_OP_INCR;
                case StencilOperation::DecrementWrap:
                    return D3D12_STENCIL_OP_DECR;
                default:
                    ALIMER_UNREACHABLE();
            }
        }
    }

    /* D3D12_Texture */
    D3D12_Texture::~D3D12_Texture()
    {
        for (auto it : shaderResourceViews)
        {
            device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, it.second);
            //device->FreeBindlessResource(bindless_srv);
        }

        for (auto it : unorderedAccessViews)
        {
            device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, it.second);
            //device->FreeBindlessResource(bindless_uav);
        }

        for (auto it : renderTargetViews)
        {
            device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, it.second);
        }

        for (auto it : depthStencilViews)
        {
            device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, it.second);
        }

        shaderResourceViews.clear();
        renderTargetViews.clear();
        depthStencilViews.clear();
        unorderedAccessViews.clear();

        if (handle != nullptr
            && allocation != nullptr)
        {
            device->DeferDestroy(handle, allocation);
        }

        handle = nullptr;
        allocation = nullptr;
    }

    IDevice* D3D12_Texture::GetDevice() const
    {
        return device;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12_Texture::GetRTV(uint32_t mipLevel, uint32_t slice, uint32_t arraySize)
    {
        if (arraySize == kAllArraySlices)
        {
            arraySize = desc.depthOrArraySize - slice;
        }
        else if (arraySize + slice > desc.depthOrArraySize)
        {
            arraySize = desc.depthOrArraySize - slice;
        }

        D3D12_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), Format::Undefined, false);

        auto it = renderTargetViews.find(key);
        if (it == renderTargetViews.end())
        {
            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = ToDXGIFormat(key.format);

            switch (desc.dimension)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case TextureDimension::Texture1D:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture1DArray:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2D:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture2DArray:
                case TextureDimension::TextureCube:
                case TextureDimension::TextureCubeArray:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = mipLevel;
                    viewDesc.Texture2DArray.FirstArraySlice = slice;
                    viewDesc.Texture2DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2DMS:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    break;
                case TextureDimension::Texture2DMSArray:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture3D:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                    viewDesc.Texture3D.MipSlice = mipLevel;
                    viewDesc.Texture3D.FirstWSlice = slice;
                    viewDesc.Texture3D.WSize = arraySize;
                    break;
                default:
                    LOGE("Texture has unsupported dimension for RTV: {}", ToString(desc.dimension));
                    return {};
            }

            D3D12_CPU_DESCRIPTOR_HANDLE rtv = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            device->GetD3DDevice()->CreateRenderTargetView(handle, &viewDesc, rtv);

            renderTargetViews[key] = rtv;
            return rtv;
        }

        return it->second;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12_Texture::GetDSV(uint32_t mipLevel, uint32_t slice, uint32_t arraySize, bool isReadOnly)
    {
        if (arraySize == kAllArraySlices)
        {
            arraySize = desc.depthOrArraySize - slice;
        }
        else if (arraySize + slice > desc.depthOrArraySize)
        {
            arraySize = desc.depthOrArraySize - slice;
        }

        D3D12_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), Format::Undefined, isReadOnly);

        auto it = depthStencilViews.find(key);
        if (it == depthStencilViews.end())
        {
            //we haven't seen this one before
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
            viewDesc.Format = ToDXGIFormat(desc.format);
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;

            if (isReadOnly)
            {
                viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
                if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
                    viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            }

            switch (desc.dimension)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case TextureDimension::Texture1D:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture1DArray:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = slice;
                    viewDesc.Texture1DArray.ArraySize = arraySize;
                    break;
                case TextureDimension::Texture2D:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MipSlice = mipLevel;
                    break;
                case TextureDimension::Texture2DArray:
                case TextureDimension::TextureCube:
                case TextureDimension::TextureCubeArray:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = mipLevel;
                    viewDesc.Texture2DArray.ArraySize = slice;
                    viewDesc.Texture2DArray.FirstArraySlice = arraySize;
                    break;
                case TextureDimension::Texture2DMS:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    break;
                case TextureDimension::Texture2DMSArray:
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                    viewDesc.Texture2DMSArray.ArraySize = arraySize;
                    break;
                default:
                    LOGE("Texture has unsupported dimension for DSV: {}", ToString(desc.dimension));
                    return {};
            }

            D3D12_CPU_DESCRIPTOR_HANDLE dsv = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            device->GetD3DDevice()->CreateDepthStencilView(handle, &viewDesc, dsv);

            depthStencilViews[key] = dsv;
            return dsv;
        }

        return it->second;
    }

    void D3D12_Texture::ApiSetName(const std::string_view& newName)
    {
        SetDebugName(handle, newName);
    }

    /* D3D12_Shader */
    void D3D12_Shader::ApiSetName(const std::string_view& newName)
    {
        ALIMER_UNUSED(newName);
    }

    IDevice* D3D12_Shader::GetDevice() const
    {
        return device;
    }

    /* D3D12_Pipeline */
    void D3D12_Pipeline::ApiSetName(const std::string_view& newName)
    {

    }

    IDevice* D3D12_Pipeline::GetDevice() const
    {
        return device;
    }

    /* D3D12_CommandList */
    D3D12_CommandList::D3D12_CommandList(D3D12_Device* device_, CommandQueue queue_)
        : device(device_)
        , queue(queue_)
    {
        for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
        {
            ThrowIfFailed(
                device->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]))
            );

            //frames[fr].resourceBuffer[cmd].init(this, 1024 * 1024); // 1 MB starting size
        }

        ThrowIfFailed(
            device->GetD3DDevice()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&handle))
        );
    }

    void D3D12_CommandList::Reset(uint32_t frameIndex)
    {
        // Start the command list in a default state:
        ThrowIfFailed(commandAllocators[frameIndex]->Reset());
        ThrowIfFailed(handle->Reset(commandAllocators[frameIndex].Get(), nullptr));

        //ID3D12DescriptorHeap* heaps[2] = {
        //    descriptorheap_res.heap_GPU.Get(),
        //    descriptorheap_sam.heap_GPU.Get()
        //};
        //handle->SetDescriptorHeaps(arraysize(heaps), heaps);
    }

    ID3D12CommandList* D3D12_CommandList::Commit()
    {
        //query_flush(cmd);
        FlushBarriers();

        ThrowIfFailed(handle->Close());

        return handle.Get();
    }

    void D3D12_CommandList::FlushBarriers()
    {
        if (barriers.empty())
            return;

        for (size_t i = 0; i < barriers.size(); ++i)
        {
            auto& barrier = barriers[i];
            if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
                queue != CommandQueue::Graphics)
            {
                // Only graphics queue can do pixel shader state:
                barrier.Transition.StateBefore &= ~D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                barrier.Transition.StateAfter &= ~D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            }
            if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION &&
                barrier.Transition.StateBefore == barrier.Transition.StateAfter)
            {
                // Remove NOP barriers:
                barrier = barriers.back();
                barriers.pop_back();
                i--;
            }
        }

        if (!barriers.empty())
        {
            handle->ResourceBarrier((UINT)barriers.size(), barriers.data());
            barriers.clear();
        }
    }

    void D3D12_CommandList::PushDebugGroup(const std::string_view& name)
    {
        auto wideName = ToUtf16(name);
        PIXBeginEvent(handle.Get(), PIX_COLOR_DEFAULT, wideName.c_str());
    }

    void D3D12_CommandList::PopDebugGroup()
    {
        PIXEndEvent(handle.Get());
    }

    void D3D12_CommandList::InsertDebugMarker(const std::string_view& name)
    {
        auto wideName = ToUtf16(name);
        PIXSetMarker(handle.Get(), PIX_COLOR_DEFAULT, wideName.c_str());
    }

    void D3D12_CommandList::BeginDefaultRenderPass(const Color& clearColor, bool clearDepth, bool clearStencil, float depth, uint8_t stencil)
    {
        RenderPassDesc passDesc;
        passDesc.colorAttachments[0].texture = device->GetCurrentBackBuffer();
        passDesc.colorAttachments[0].loadAction = LoadAction::Clear;
        passDesc.colorAttachments[0].clearColor = clearColor;
        passDesc.colorAttachments[0].initialState = ResourceStates::Unknown;
        passDesc.colorAttachments[0].finalState = ResourceStates::Present;

        auto depthStencilTexture = device->GetBackBufferDepthStencilTexture();
        if (depthStencilTexture != nullptr)
        {
            passDesc.depthStencilAttachment.texture = depthStencilTexture;
            if (clearDepth)
            {
                passDesc.depthStencilAttachment.depthLoadAction = LoadAction::Clear;
                passDesc.depthStencilAttachment.clearDepth = depth;
            }

            if (clearStencil)
            {
                passDesc.depthStencilAttachment.stencilLoadAction = LoadAction::Clear;
                passDesc.depthStencilAttachment.clearStencil = stencil;
            }
        }

        BeginRenderPass(passDesc);
    }

    void D3D12_CommandList::BeginRenderPass(const RenderPassDesc& desc)
    {
        currentPass = desc;
        uint32_t width = UINT32_MAX;
        uint32_t height = UINT32_MAX;

        uint32_t RTVCount = 0;
        std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxColorAttachments> RTVs;
        D3D12_CPU_DESCRIPTOR_HANDLE DSV = {};

        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = desc.colorAttachments[i];
            if (attachment.texture == nullptr)
                break;

            auto d3d12Texture = checked_cast<D3D12_Texture*>(attachment.texture);
            const TextureDesc& textureDesc = d3d12Texture->GetDesc();

            const uint32_t mipLevel = attachment.mipLevel;
            const uint32_t slice = attachment.slice;

            width = Min(width, std::max(1u, textureDesc.width >> mipLevel));
            height = Min(height, std::max(1u, textureDesc.height >> mipLevel));

            RTVs[RTVCount] = d3d12Texture->GetRTV(mipLevel, slice, 1);

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = d3d12Texture->GetHandle();
            barrier.Transition.StateBefore = ToD3D12(attachment.initialState);
            barrier.Transition.StateAfter = ToD3D12(attachment.finalState);
            if (attachment.finalState == ResourceStates::Present)
            {
                barrier.Transition.StateAfter = ToD3D12(ResourceStates::RenderTarget);
            }

            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers.push_back(barrier);
            FlushBarriers();

            switch (attachment.loadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    handle->ClearRenderTargetView(RTVs[RTVCount], &attachment.clearColor.r, 0, nullptr);
                    break;

                case LoadAction::DontCare:
                    handle->DiscardResource(d3d12Texture->GetHandle(), nullptr);
                    break;
            }

            RTVCount++;
        }

        if (desc.depthStencilAttachment.texture != nullptr)
        {
            const RenderPassDepthStencilAttachment& attachment = desc.depthStencilAttachment;

            auto d3d11Texture = checked_cast<D3D12_Texture*>(attachment.texture);
            const TextureDesc& textureDesc = d3d11Texture->GetDesc();

            width = Min(width, std::max(1u, textureDesc.width >> attachment.mipLevel));
            height = Min(height, std::max(1u, textureDesc.height >> attachment.mipLevel));

            DSV = d3d11Texture->GetDSV(attachment.mipLevel, attachment.slice, 1, desc.depthStencilAttachment.depthStencilReadOnly);

            D3D12_CLEAR_FLAGS clearFlags = {};

            switch (desc.depthStencilAttachment.depthLoadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                    break;

                case LoadAction::DontCare:
                    //handle->DiscardResource(DSV);
                    break;
            }

            switch (desc.depthStencilAttachment.stencilLoadAction)
            {
                default:
                case LoadAction::Load:
                    break;

                case LoadAction::Clear:
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                    break;

                case LoadAction::DontCare:
                    //handle->DiscardResource(DSV);
                    break;
            }

            if (clearFlags != 0)
            {
                handle->ClearDepthStencilView(DSV, clearFlags, desc.depthStencilAttachment.clearDepth, desc.depthStencilAttachment.clearStencil, 0, nullptr);
            }
        }

        handle->OMSetRenderTargets(RTVCount, RTVs.data(), FALSE, &DSV);

        // The viewport and scissor default to cover all of the attachments
        const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
        const D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

        handle->RSSetViewports(1, &viewport);
        handle->RSSetScissorRects(1, &scissorRect);
    }

    void D3D12_CommandList::EndRenderPass()
    {
        for (uint32_t i = 0; i < kMaxColorAttachments; i++)
        {
            const RenderPassColorAttachment& attachment = currentPass.colorAttachments[i];
            if (attachment.texture == nullptr)
                break;

            auto d3d11Texture = checked_cast<D3D12_Texture*>(attachment.texture);

            switch (attachment.storeAction)
            {
                case StoreAction::Resolve:
                case StoreAction::StoreAndResolve:
                {
                    auto resolveTexture = checked_cast<D3D12_Texture*>(attachment.resolveTexture);
                    uint32_t dstSubresource = D3D12CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, 0, resolveTexture->desc.mipLevels, resolveTexture->desc.depthOrArraySize);
                    uint32_t srcSubresource = D3D12CalcSubresource(attachment.mipLevel, attachment.slice, 0, d3d11Texture->desc.mipLevels, resolveTexture->desc.depthOrArraySize);
                    handle->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                    break;
                }

                default:
                    if (attachment.finalState == ResourceStates::Present)
                    {
                        D3D12_RESOURCE_BARRIER barrier = {};
                        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                        barrier.Transition.pResource = d3d11Texture->handle;
                        barrier.Transition.StateBefore = (attachment.initialState == ResourceStates::Unknown) ? ToD3D12(ResourceStates::RenderTarget) : ToD3D12(attachment.initialState);
                        barrier.Transition.StateAfter = ToD3D12(attachment.finalState);
                        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                        barriers.push_back(barrier);
                    }
                    break;
            }
        }

        if (currentPass.depthStencilAttachment.texture != nullptr)
        {
            const RenderPassDepthStencilAttachment& attachment = currentPass.depthStencilAttachment;
            auto d3d11Texture = checked_cast<D3D12_Texture*>(attachment.texture);

            switch (attachment.depthStoreAction)
            {
                case StoreAction::DontCare:
                {
                    //auto DSV = d3d11Texture->GetDSV(attachment.mipLevel, attachment.slice, 1);
                    //handle->DiscardView(DSV);
                    break;
                }

                case StoreAction::Resolve:
                case StoreAction::StoreAndResolve:
                {
                    auto resolveTexture = checked_cast<D3D12_Texture*>(attachment.resolveTexture);
                    uint32_t dstSubresource = D3D12CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, 0, resolveTexture->desc.mipLevels, resolveTexture->desc.depthOrArraySize);
                    uint32_t srcSubresource = D3D12CalcSubresource(attachment.mipLevel, attachment.slice, 0, d3d11Texture->desc.mipLevels, resolveTexture->desc.depthOrArraySize);
                    handle->ResolveSubresource(resolveTexture->handle, dstSubresource, d3d11Texture->handle, srcSubresource, d3d11Texture->dxgiFormat);
                    break;
                }

                default:
                    break;
            }
        }
    }

    void D3D12_CommandList::SetPipeline(_In_ IPipeline* pipeline)
    {
        D3D12_Pipeline* d3dPipeline = checked_cast<D3D12_Pipeline*>(pipeline);
        BindRenderPipeline(d3dPipeline);
    }

    void D3D12_CommandList::BindRenderPipeline(const D3D12_Pipeline* pipeline)
    {
        //context->VSSetShader(pipeline->vertex, nullptr, 0);
        //context->HSSetShader(pipeline->hull, nullptr, 0);
        //context->DSSetShader(pipeline->domain, nullptr, 0);
        //context->GSSetShader(pipeline->geometry, nullptr, 0);
        //context->PSSetShader(pipeline->pixel, nullptr, 0);
        //
        //context->IASetPrimitiveTopology(pipeline->primitiveTopology);
        //context->IASetInputLayout(pipeline->inputLayout);
        //
        //float blendColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        //context->OMSetBlendState(pipeline->blendState, blendColor, D3D11_DEFAULT_SAMPLE_MASK);
        //context->OMSetDepthStencilState(pipeline->depthStencilState, D3D11_DEFAULT_STENCIL_REFERENCE);
        //context->RSSetState(pipeline->rasterizerState);
    }

    void D3D12_CommandList::Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
    {
        handle->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
    }

    /* D3D12_Device */
    bool D3D12_Device::IsAvailable()
    {
        static bool available_initialized = false;
        static bool available = false;

        if (available_initialized) {
            return available;
        }

        available_initialized = true;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        HMODULE dxgiDLL = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        HMODULE d3d12DLL = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        //HMODULE dxcompiler = LoadLibraryW(L"dxcompiler.dll");

        if (dxgiDLL == nullptr ||
            d3d12DLL == nullptr)
        {
            return false;
        }

        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(dxgiDLL, "CreateDXGIFactory2");
        if (CreateDXGIFactory2 == nullptr)
        {
            return false;
        }

        DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)GetProcAddress(dxgiDLL, "DXGIGetDebugInterface1");

        D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12DLL, "D3D12GetDebugInterface");
        D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12DLL, "D3D12CreateDevice");
        if (!D3D12CreateDevice) {
            return false;
        }

        D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(d3d12DLL, "D3D12SerializeVersionedRootSignature");
        if (!D3D12SerializeVersionedRootSignature) {
            return false;
        }
#else
        HMODULE dxcompiler = LoadPackagedLibrary(L"dxcompiler.dll", 0);
#endif

        //if (dxcompiler)
        //{
        //    DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(dxcompiler, "DxcCreateInstance");
        //    ALIMER_ASSERT(DxcCreateInstance != nullptr);
        //}

        if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
        {
            available = true;
            return true;
        }

        return false;
    }

    D3D12_Device::D3D12_Device(ValidationMode validationMode)
    {
        if (validationMode != ValidationMode::Disabled)
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            RefCountPtr<ID3D12Debug> d3d12Debug;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.GetAddressOf()))))
            {
                d3d12Debug->EnableDebugLayer();

                if (validationMode == ValidationMode::GPU)
                {
                    RefCountPtr<ID3D12Debug1> d3d12Debug1;
                    if (SUCCEEDED(d3d12Debug->QueryInterface(IID_PPV_ARGS(&d3d12Debug1))))
                    {
                        d3d12Debug1->SetEnableGPUBasedValidation(TRUE);
                        d3d12Debug1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                    }

                    RefCountPtr<ID3D12Debug2> d3d12Debug2;
                    if (SUCCEEDED(d3d12Debug->QueryInterface(IID_PPV_ARGS(&d3d12Debug2))))
                    {
                        d3d12Debug2->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                    }
                }
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }

#if defined(_DEBUG)
            RefCountPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

                DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
                {
                    80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
                };
                DXGI_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
                filter.DenyList.pIDList = hide;
                dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
            }
#endif
        }

        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        // Determines whether tearing support is available for fullscreen borderless windows.
        {
            RefCountPtr<IDXGIFactory5> dxgiFactory5;
            if (SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory5))))
            {
                BOOL supported = 0;
                if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
                {
                    tearingSupported = (supported != 0);
                }
            }
        }
    }

    D3D12_Device::~D3D12_Device()
    {
        shuttingDown = true;

        // CPU Descriptor Heaps
        resourceDescriptorAllocator.Shutdown();
        samplerDescriptorAllocator.Shutdown();
        rtvDescriptorAllocator.Shutdown();
        dsvDescriptorAllocator.Shutdown();


        for (size_t i = 0; i < backBuffers.size(); ++i)
        {
            backBuffers[i].Reset();
        }
        backBuffers.clear();
        depthStencilTexture.Reset();
        swapChain.Reset();

        frameCount = UINT64_MAX;
        ProcessDeletionQueue();
        frameCount = 0;

        // Allocator.
        if (allocator != nullptr)
        {
            D3D12MA::Stats stats;
            allocator->CalculateStats(&stats);

            if (stats.Total.UsedBytes > 0)
            {
                LOGI("Total device memory leaked: {} bytes.", stats.Total.UsedBytes);
            }

            allocator->Release();
            allocator = nullptr;
        }
    }

    void D3D12_Device::ProcessDeletionQueue()
    {
        AcquireSRWLockExclusive(&destroyMutex);

        while (!deferredAllocations.empty())
        {
            if (deferredAllocations.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deferredAllocations.front();
                deferredAllocations.pop_front();
                item.first->Release();
            }
            else
            {
                break;
            }
        }

        while (!deferredReleases.empty())
        {
            if (deferredReleases.front().second + kMaxFramesInFlight < frameCount)
            {
                auto item = deferredReleases.front();
                deferredReleases.pop_front();
                item.first->Release();
            }
            else
            {
                break;
            }
        }

#if TODO
        while (!destroyedBindlessResources.empty())
        {
            if (destroyedBindlessResources.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessResources.front().first;
                destroyedBindlessResources.pop_front();
                freeBindlessResources.push_back(index);
            }
            else
            {
                break;
            }
        }

        while (!destroyedBindlessSamplers.empty())
        {
            if (destroyedBindlessSamplers.front().second + kMaxFramesInFlight < frameCount)
            {
                uint32_t index = destroyedBindlessSamplers.front().first;
                destroyedBindlessSamplers.pop_front();
                freeBindlessSamplers.push_back(index);
            }
            else
            {
                break;
            }
        }
#endif // TODO

        ReleaseSRWLockExclusive(&destroyMutex);
    }

    bool D3D12_Device::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        // Get adapter, create device + caps + allocator
        {
            RefCountPtr<IDXGIAdapter1> adapter;
            GetAdapter(adapter.GetAddressOf());

            // Create the DX12 API device object.
            HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3dDevice));
            ThrowIfFailed(hr);

            // Configure debug device (if active).
            if (presentationParameters.validationMode != ValidationMode::Disabled)
            {
                RefCountPtr<ID3D12InfoQueue> d3d12InfoQueue;
                if (SUCCEEDED(d3dDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue))))
                {
#ifdef _DEBUG
                    d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                    d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
#endif
                    D3D12_MESSAGE_ID hide[] =
                    {
                        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,

                        D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                        D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                        //D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE
                    };
                    D3D12_INFO_QUEUE_FILTER filter = {};
                    filter.DenyList.NumIDs = _countof(hide);
                    filter.DenyList.pIDList = hide;
                    d3d12InfoQueue->AddStorageFilterEntries(&filter);

                    // Break on DEVICE_REMOVAL_PROCESS_AT_FAULT
                    d3d12InfoQueue->SetBreakOnID(D3D12_MESSAGE_ID_DEVICE_REMOVAL_PROCESS_AT_FAULT, TRUE);
                }
            }

            // Init capabilities and features.
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(adapter->GetDesc1(&adapterDesc));

            // Init feature check
            CD3DX12FeatureSupport features;
            ThrowIfFailed(features.Init(d3dDevice.Get()));

            featureLevel = features.MaxSupportedFeatureLevel();

            D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
            allocatorDesc.pDevice = d3dDevice.Get();
            allocatorDesc.pAdapter = adapter.Get();

            if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &allocator)))
            {
                return false;
            }

            LOGI("Create Direct3D12 device {} with adapter: VID:{:#04x}, PID:{:#04x} - {}",
                ToString(featureLevel),
                adapterDesc.VendorId,
                adapterDesc.DeviceId,
                ToUtf8(adapterDesc.Description)
            );
        }

        // Create command queues + fences
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.NodeMask = 1;
            ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queues[(uint8_t)CommandQueue::Graphics].queue)));
            ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&queues[(uint8_t)CommandQueue::Graphics].fence)));

            // Compute queue.
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            ThrowIfFailed(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queues[(uint8_t)CommandQueue::Compute].queue)));
            ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&queues[(uint8_t)CommandQueue::Compute].fence)));
        }

        // Descriptor allocators
        {
            resourceDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);
            samplerDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024);
            rtvDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024);
            dsvDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256);
        }

        // Create frame fences per queue
        {
            for (uint32_t i = 0; i < kMaxFramesInFlight; ++i)
            {
                for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
                {
                    ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queues[queue].frameFence[i])));
                }
            }
        }

        // Create SwapChain
        {
            swapChainDesc = {};
            swapChainDesc.Width = presentationParameters.backBufferWidth;
            swapChainDesc.Height = presentationParameters.backBufferHeight;
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = presentationParameters.backBufferCount;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

            RefCountPtr<IDXGISwapChain1> tempSwapChain;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            fullScreenDesc = {};
            fullScreenDesc.RefreshRate.Numerator = 0;
            fullScreenDesc.RefreshRate.Denominator = 1;
            fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
            fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fullScreenDesc.Windowed = !presentationParameters.isFullScreen;

            // Create a SwapChain from a Win32 window.
            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
                GetGraphicsQueue(),
                static_cast<HWND>(window->GetPlatformHandle()),
                &swapChainDesc,
                &fullScreenDesc,
                nullptr,
                tempSwapChain.GetAddressOf()
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(dxgiFactory->MakeWindowAssociation(static_cast<HWND>(window->GetPlatformHandle()), DXGI_MWA_NO_ALT_ENTER));
#else
#endif

            ThrowIfFailed(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)));
        }

        depthStencilFormat = presentationParameters.depthStencilFormat;

        AfterReset();

        return true;
    }

    void D3D12_Device::WaitIdle()
    {
        // TODO:
    }

    void D3D12_Device::DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation)
    {
        AcquireSRWLockExclusive(&destroyMutex);

        if (shuttingDown)
        {
            resource->Release();
            if (allocation != nullptr)
            {
                allocation->Release();
            }

            ReleaseSRWLockExclusive(&destroyMutex);
            return;
        }

        deferredReleases.push_back(std::make_pair(resource, frameCount));
        if (allocation != nullptr)
        {
            deferredAllocations.push_back(std::make_pair(allocation, frameCount));
        }
        ReleaseSRWLockExclusive(&destroyMutex);
    }

    ICommandList* D3D12_Device::BeginCommandList(CommandQueue queue)
    {
        uint8_t cmd = commandListCount.fetch_add(1);

        ALIMER_ASSERT(cmd < kMaxCommandLists);
        commandListMeta[cmd].queue = queue;
        commandListMeta[cmd].waits.clear();

        if (GetCommandList(cmd) == nullptr)
        {
            commandLists[cmd][(uint8_t)queue] = std::make_unique<D3D12_CommandList>(this, queue);
        }

        GetCommandList(cmd)->Reset(frameIndex);

        return GetCommandList(cmd);
    }

    void D3D12_Device::SubmitCommandLists()
    {
        CommandQueue submitQueue = CommandQueue::Count;

        uint8_t cmd_last = commandListCount.load();
        commandListCount.store(0);
        for (uint8_t cmd = 0; cmd < cmd_last; ++cmd)
        {
            ID3D12CommandList* commandList = GetCommandList(cmd)->Commit();

            const CommandListMetadata& meta = commandListMeta[cmd];
            if (submitQueue == CommandQueue::Count)
            {
                submitQueue = meta.queue;
            }

            if (meta.queue != submitQueue || !meta.waits.empty()) // new queue type or wait breaks submit batch
            {
                // submit previous cmd batch:
                if (queues[(uint8_t)submitQueue].submitCount > 0)
                {
                    queues[(uint8_t)submitQueue].queue->ExecuteCommandLists(
                        queues[(uint8_t)submitQueue].submitCount,
                        queues[(uint8_t)submitQueue].submitCommandLists
                    );
                    queues[(uint8_t)submitQueue].submitCount = 0;
                }

                // signal status in case any future waits needed:
                ThrowIfFailed(queues[(uint8_t)submitQueue].queue->Signal(
                    queues[(uint8_t)submitQueue].fence.Get(),
                    kMaxFramesInFlight * kMaxCommandLists + (uint64_t)cmd
                ));

                submitQueue = meta.queue;

                for (auto& wait : meta.waits)
                {
                    // record wait for signal on a previous submit:
                    const CommandListMetadata& waitMeta = commandListMeta[wait];
                    ThrowIfFailed(queues[(uint8_t)submitQueue].queue->Wait(
                        queues[(uint8_t)waitMeta.queue].fence.Get(),
                        kMaxFramesInFlight * kMaxCommandLists + (uint64_t)wait
                    ));
                }
            }

            ALIMER_ASSERT(submitQueue < CommandQueue::Count);
            queues[(uint8)submitQueue].submitCommandLists[queues[(uint8)submitQueue].submitCount++] = commandList;
        }

        // submit last cmd batch:
        ALIMER_ASSERT(submitQueue < CommandQueue::Count);
        ALIMER_ASSERT(queues[(uint8)submitQueue].submitCount > 0);
        queues[(uint8)submitQueue].queue->ExecuteCommandLists(
            queues[(uint8)submitQueue].submitCount,
            queues[(uint8)submitQueue].submitCommandLists
        );
        queues[(uint8)submitQueue].submitCount = 0;

        // Mark the completion of queues for this frame:
        for (uint8 queue = 0; queue < (uint8)CommandQueue::Count; ++queue)
        {
            ThrowIfFailed(queues[queue].queue->Signal(queues[queue].frameFence[frameIndex].Get(), 1));
        }
    }

    ICommandList* D3D12_Device::BeginFrame()
    {
        if (deviceLost)
        {
            return nullptr;
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc;
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
        if (SUCCEEDED(swapChain->GetDesc1(&newSwapChainDesc)) &&
            SUCCEEDED(swapChain->GetFullscreenDesc(&newFullScreenDesc)))
        {
            if (fullScreenDesc.Windowed != newFullScreenDesc.Windowed)
            {
            }
        }
#endif

        return BeginCommandList();
    }

    void D3D12_Device::EndFrame()
    {
        SubmitCommandLists();

        UINT presentFlags = 0;
        if (!vsyncEnabled && fullScreenDesc.Windowed && tearingSupported)
        {
            presentFlags |= DXGI_PRESENT_ALLOW_TEARING;
        }

        HRESULT hr = swapChain->Present(vsyncEnabled ? 1 : 0, presentFlags);

        // If the device was removed either by a disconnection or a driver upgrade, we
        // must recreate all device resources.
        if (hr == DXGI_ERROR_DEVICE_REMOVED
            || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice->GetDeviceRemovedReason() : hr));
            OutputDebugStringA(buff);
#endif
            HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);

            // From here, we begin a new frame, this affects GetFrameResources()!
            frameCount++;
            frameIndex = frameCount % kMaxFramesInFlight;

            // Begin next frame
            {
                // Initiate stalling CPU when GPU is not yet finished with next frame:
                for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
                {
                    if (frameCount >= kMaxFramesInFlight && queues[queue].frameFence[frameIndex]->GetCompletedValue() < 1)
                    {
                        // NULL event handle will simply wait immediately:
                        //	https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-seteventoncompletion#remarks
                        ThrowIfFailed(queues[queue].frameFence[frameIndex]->SetEventOnCompletion(1, nullptr));
                    }
                    ThrowIfFailed(queues[queue].frameFence[frameIndex]->Signal(0));
                }

                // Descriptor heaps' progress is recorded by the GPU:
                //descriptorheap_res.fenceValue = descriptorheap_res.allocationOffset.load();
                //hr = queues[QUEUE_GRAPHICS].queue->Signal(descriptorheap_res.fence.Get(), descriptorheap_res.fenceValue);
                //assert(SUCCEEDED(hr));
                //descriptorheap_res.cached_completedValue = descriptorheap_res.fence->GetCompletedValue();
                //descriptorheap_sam.fenceValue = descriptorheap_sam.allocationOffset.load();
                //hr = queues[QUEUE_GRAPHICS].queue->Signal(descriptorheap_sam.fence.Get(), descriptorheap_sam.fenceValue);
                //assert(SUCCEEDED(hr));
                //descriptorheap_sam.cached_completedValue = descriptorheap_sam.fence->GetCompletedValue();

                ProcessDeletionQueue();
            }

            if (!dxgiFactory->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));
            }
        }
    }

    void D3D12_Device::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        AfterReset();
    }

    void D3D12_Device::AfterReset()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(swapChain->GetDesc1(&swapChainDesc));

        const TextureDesc backBufferTextureDesc = TextureDesc::Tex2D(Format::BGRA8UNorm,
            swapChainDesc.Width, swapChainDesc.Height, 1, 1, TextureUsage::RenderTarget
        );

        backBuffers.resize(swapChainDesc.BufferCount);
        for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
        {
            RefCountPtr<ID3D12Resource> d3d12BackBuffer;
            ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&d3d12BackBuffer)));

            backBuffers[i] = CreateExternalTexture(d3d12BackBuffer.Get(), backBufferTextureDesc);
        }

        backBufferWidth = swapChainDesc.Width;
        backBufferHeight = swapChainDesc.Height;

        if (depthStencilFormat != Format::Undefined)
        {
            TextureDesc depthStencilTextureDesc = TextureDesc::Tex2D(depthStencilFormat, backBufferWidth, backBufferHeight, 1, 1, TextureUsage::RenderTarget);
            depthStencilTextureDesc.initialState = ResourceStates::DepthWrite;
            depthStencilTexture = CreateTexture(depthStencilTextureDesc);
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12_Device::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        switch (type)
        {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                return resourceDescriptorAllocator.Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return samplerDescriptorAllocator.Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
                return rtvDescriptorAllocator.Allocate();
            case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
                return dsvDescriptorAllocator.Allocate();
            default:
                ALIMER_UNREACHABLE();
                return {};
        }
    }

    void D3D12_Device::FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        if (!handle.ptr)
            return;

        switch (type)
        {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                resourceDescriptorAllocator.Free(handle);
                break;
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                samplerDescriptorAllocator.Free(handle);
                break;
            case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
                rtvDescriptorAllocator.Free(handle);
                break;
            case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
                dsvDescriptorAllocator.Free(handle);
                break;
            default:
                ALIMER_UNREACHABLE();
                break;
        }
    }

    uint32_t D3D12_Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
        switch (type)
        {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                return resourceDescriptorAllocator.descriptorSize;
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return samplerDescriptorAllocator.descriptorSize;
            case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
                return rtvDescriptorAllocator.descriptorSize;
            case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
                return dsvDescriptorAllocator.descriptorSize;
            default:
                ALIMER_UNREACHABLE();
        }
    }

    TextureHandle D3D12_Device::CreateTextureCore(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        auto result = new D3D12_Texture();
        result->device = this;
        result->desc = desc;

        if (desc.mipLevels == 0)
        {
            result->desc.mipLevels = (uint32_t)log2(std::max(desc.width, desc.height)) + 1;
        }


        result->dxgiFormat = ToDXGIFormat(desc.format);

        if (nativeHandle != nullptr)
        {
            result->handle = (ID3D12Resource*)nativeHandle;
            return TextureHandle::Create(result);
        }

        DXGI_FORMAT format = result->dxgiFormat;

        D3D12MA::ALLOCATION_DESC allocationDesc{};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Alignment = 0;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.MipLevels = desc.mipLevels;
        resourceDesc.Format = ToDXGIFormat(desc.format);
        resourceDesc.SampleDesc.Count = desc.sampleCount;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        switch (desc.dimension)
        {
            case TextureDimension::Texture1D:
            case TextureDimension::Texture1DArray:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                resourceDesc.DepthOrArraySize = UINT16(desc.depthOrArraySize);
                break;

            case TextureDimension::Texture2D:
            case TextureDimension::Texture2DArray:
            case TextureDimension::TextureCube:
            case TextureDimension::TextureCubeArray:
            case TextureDimension::Texture2DMS:
            case TextureDimension::Texture2DMSArray:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                resourceDesc.DepthOrArraySize = UINT16(desc.depthOrArraySize);
                break;

            case TextureDimension::Texture3D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                resourceDesc.DepthOrArraySize = UINT16(desc.depthOrArraySize);
                break;

            default:
                break;
        }

        D3D12_CLEAR_VALUE clearValue = {};
        D3D12_CLEAR_VALUE* pClearValue = nullptr;

        ResourceStates bestInitialState = ResourceStates::ShaderResource;
        if (Any(desc.usage, TextureUsage::RenderTarget))
        {
            // Render targets and Depth/Stencil targets are always committed resources
            allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;

            clearValue.Format = resourceDesc.Format;

            if (IsDepthStencilFormat(desc.format))
            {
                bestInitialState = ResourceStates::DepthWrite;
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                if (!Any(desc.usage, TextureUsage::ShaderRead))
                {
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                }

                clearValue.DepthStencil.Depth = 1.0f;
            }
            else
            {
                bestInitialState = ResourceStates::RenderTarget;
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }

            pClearValue = &clearValue;
        }

        if ((desc.usage & TextureUsage::ShaderWrite) != 0)
        {
            // Remove unsupported flags.
            resourceDesc.Flags &= ~D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            resourceDesc.Flags &= ~D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        // If depth and either shader read, set to typeless
        if (IsDepthFormat(desc.format)
            && Any(desc.usage, TextureUsage::ShaderRead))
        {
            resourceDesc.Format = GetTypelessFormatFromDepthFormat(desc.format);
            pClearValue = nullptr;
        }

        D3D12_RESOURCE_STATES state;
        if (initialData != nullptr)
        {
            state = D3D12_RESOURCE_STATE_COMMON;
        }
        else
        {
            state = desc.initialState != ResourceStates::Unknown ? ToD3D12(desc.initialState) : ToD3D12(bestInitialState);
        }

        HRESULT hr = allocator->CreateResource(&allocationDesc,
            &resourceDesc,
            state,
            pClearValue,
            &result->allocation,
            IID_PPV_ARGS(&result->handle)
        );

        if (FAILED(hr))
        {
            delete result;
            return nullptr;
        }

        if (result->handle)
            return TextureHandle::Create(result);

        delete result;
        return nullptr;
    }

    ShaderHandle D3D12_Device::CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
        auto byteCode = CompileShader(stage, source, entryPoint);
        if (byteCode.empty())
            return nullptr;

        RefCountPtr<D3D12_Shader> shader = RefCountPtr<D3D12_Shader>::Create(new D3D12_Shader());
        shader->device = this;
        shader->bytecode.resize(byteCode.size());
        memcpy(shader->bytecode.data(), byteCode.data(), byteCode.size());
        return shader;
    }

    std::vector<uint8_t> D3D12_Device::CompileShader(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
#if defined(ALIMER_DISABLE_SHADER_COMPILER)
        return {};
#else
        if (!LoadShaderCompiler())
            return {};

        UINT compileFlags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;

#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        std::string profile;
        switch (stage)
        {
            // clang-format off
            case ShaderStages::Vertex:           profile = "vs"; break;
            case ShaderStages::Hull:             profile = "hs"; break;
            case ShaderStages::Domain:           profile = "ds"; break;
            case ShaderStages::Geometry:         profile = "gs"; break;
            case ShaderStages::Pixel:            profile = "ps"; break;
            case ShaderStages::Compute:          profile = "cs"; break;
            default:
                ALIMER_UNREACHABLE();
                return {};
        }

        const uint32_t shaderModelMajor = 5;
        const uint32_t shaderModelMinor = 0;

        profile += "_";
        profile += std::to_string(shaderModelMajor);
        profile += "_";
        profile += std::to_string(shaderModelMinor);

        RefCountPtr<ID3DBlob> output;
        RefCountPtr<ID3DBlob> errors_or_warnings;

        HRESULT hr = D3DCompile(
            source.c_str(),                     /* pSrcData */
            source.length(),                    /* SrcDataSize */
            nullptr,                            /* pSourceName */
            NULL,                               /* pDefines */
            D3D_COMPILE_STANDARD_FILE_INCLUDE,  /* pInclude */
            entryPoint.c_str(),                 /* pEntryPoint */
            profile.c_str(),                    /* pTarget */
            compileFlags,                       /* Flags1 */
            0,                                  /* Flags2 */
            &output,                            /* ppCode */
            &errors_or_warnings);               /* ppErrorMsgs */

        if (errors_or_warnings)
        {
            LOGE((LPCSTR)errors_or_warnings->GetBufferPointer());
        }

        if (FAILED(hr))
        {
            return {};
        }

        std::vector<uint8_t> byteCode(output->GetBufferSize());
        //shader->bytecode.resize(byteCode.size());
        memcpy(byteCode.data(), output->GetBufferPointer(), output->GetBufferSize());
        return byteCode;
#endif
    }

#if !defined(ALIMER_DISABLE_SHADER_COMPILER)
    bool D3D12_Device::LoadShaderCompiler()
    {
#if (defined(WINAPI_FAMILY_PARTITION) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP))
        return true;
#else
        /* load DLL on demand */
        if (D3DCompiler == nullptr && !D3DCompiler_LoadFailed)
        {
            D3DCompiler = LoadLibraryW(L"D3DCompiler_47.dll");
            if (D3DCompiler == nullptr)
            {
                /* don't attempt to load missing DLL in the future */
                LOGD("Direct3D11: Failed to load D3DCompiler_47.dll!");
                D3DCompiler_LoadFailed = true;
                return false;
            }

            /* look up function pointers */
            D3DCompile = (pD3DCompile)(void*)GetProcAddress(D3DCompiler, "D3DCompile");
            ALIMER_ASSERT(D3DCompile != nullptr);
        }

        return D3DCompiler != nullptr;
#endif
    }
#endif

    PipelineHandle D3D12_Device::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        RefCountPtr<D3D12_Pipeline> pipeline = RefCountPtr<D3D12_Pipeline>::Create(new D3D12_Pipeline());
        pipeline->device = this;

        pipeline->shaderStages = ShaderStages::None;
        if (desc.vertex)
        {
            //pipeline->vertex = checked_cast<D3D12_Shader*>(desc.vertex)->VS;
            pipeline->shaderStages |= ShaderStages::Vertex;
        }

        if (desc.hull)
        {
            //pipeline->hull = checked_cast<D3D12_Shader*>(desc.hull)->HS;
            pipeline->shaderStages |= ShaderStages::Hull;
        }

        if (desc.domain)
        {
            //pipeline->domain = checked_cast<D3D12_Shader*>(desc.domain)->DS;
            pipeline->shaderStages |= ShaderStages::Domain;
        }

        if (desc.geometry)
        {
            //pipeline->geometry = checked_cast<D3D12_Shader*>(desc.geometry)->GS;
            pipeline->shaderStages |= ShaderStages::Geometry;
        }

        if (desc.pixel)
        {
            // pipeline->pixel = checked_cast<D3D12_Shader*>(desc.pixel)->PS;
            pipeline->shaderStages |= ShaderStages::Pixel;
        }

        //pipeline->blendState = GetBlendState(desc.blendState);
        //pipeline->depthStencilState = GetDepthStencilState(desc.depthStencilState);
        //pipeline->rasterizerState = GetRasterizerState(desc.rasterizerState);

        pipeline->primitiveTopology = ConvertPrimitiveTopology(desc.primitiveTopology, desc.patchControlPoints);

        return pipeline;
    }

    void D3D12_Device::GetAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        RefCountPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        RefCountPtr<IDXGIFactory6> dxgiFactory6;
        if (SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory6))))
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(dxgiFactory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                    IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
#ifdef _DEBUG
                    wchar_t buff[256] = {};
                    swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                    OutputDebugStringW(buff);
#endif
                    break;
                }
            }
        }
#endif
        if (!adapter)
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(dxgiFactory->EnumAdapters1(
                    adapterIndex,
                    adapter.ReleaseAndGetAddressOf()));
                adapterIndex++)
            {
                DXGI_ADAPTER_DESC1 desc;
                ThrowIfFailed(adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
#ifdef _DEBUG
                    wchar_t buff[256] = {};
                    swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                    OutputDebugStringW(buff);
#endif
                    break;
                }
            }
        }

#if !defined(NDEBUG)
        if (!adapter)
        {
            // Try WARP12 instead
            if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
            {
                throw std::runtime_error("WARP12 not available. Enable the 'Graphics Tools' optional feature");
            }

            OutputDebugStringA("Direct3D Adapter - WARP12\n");
        }
#endif

        if (!adapter)
        {
            throw std::runtime_error("No Direct3D 12 device found");
        }

        *ppAdapter = adapter.Detach();
    }

    void D3D12_Device::HandleDeviceLost()
    {
        // TODO
    }

    DeviceHandle CreateD3D12Device(alimer::Window* window, const PresentationParameters& presentationParameters)
    {
        if (!D3D12_Device::IsAvailable())
        {
            return nullptr;
        }

        auto device = new D3D12_Device(presentationParameters.validationMode);
        if (device->Initialize(window, presentationParameters))
        {
            return DeviceHandle::Create(device);
        }

        delete device;
        return nullptr;
    }
}

#endif /* defined(ALIMER_RHI_D3D11) */
