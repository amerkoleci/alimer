// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"
#if defined(ALIMER_RHI_D3D12) && defined(TODO)
#include "Window.h"
#include "Core/StringUtils.h"
#include "Core/Log.h"
#include "RHI_D3D12.h"

#include "directx/d3dx12.h"

#include <dxcapi.h>
#include "directx/d3d12shader.h"
#include <pix.h>

namespace Alimer::rhi
{
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
        static_assert(sizeof(Alimer::Viewport) == sizeof(D3D12_VIEWPORT), "Size mismatch");
        static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D12_VIEWPORT, Width), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D12_VIEWPORT, Height), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "Layout mismatch");

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

        inline void SetDebugName(ID3D12Object* object, const std::string_view& name)
        {
            auto wideName = ToUtf16(name);
            ThrowIfFailed(object->SetName(wideName.c_str()));
        }

        [[nodiscard]] constexpr DXGI_FORMAT ToDXGIFormat(PixelFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case PixelFormat::R8UNorm:  return DXGI_FORMAT_R8_UNORM;
                case PixelFormat::R8SNorm:  return DXGI_FORMAT_R8_SNORM;
                case PixelFormat::R8UInt:   return DXGI_FORMAT_R8_UINT;
                case PixelFormat::R8SInt:   return DXGI_FORMAT_R8_SINT;
                    // 16-bit formats
                case PixelFormat::R16UNorm:     return DXGI_FORMAT_R16_UNORM;
                case PixelFormat::R16SNorm:     return DXGI_FORMAT_R16_SNORM;
                case PixelFormat::R16UInt:      return DXGI_FORMAT_R16_UINT;
                case PixelFormat::R16SInt:      return DXGI_FORMAT_R16_SINT;
                case PixelFormat::R16Float:     return DXGI_FORMAT_R16_FLOAT;
                case PixelFormat::RG8UNorm:     return DXGI_FORMAT_R8G8_UNORM;
                case PixelFormat::RG8SNorm:     return DXGI_FORMAT_R8G8_SNORM;
                case PixelFormat::RG8UInt:      return DXGI_FORMAT_R8G8_UINT;
                case PixelFormat::RG8SInt:      return DXGI_FORMAT_R8G8_SINT;
                    // Packed 16-Bit Pixel Formats
                case PixelFormat::BGRA4UNorm:       return DXGI_FORMAT_B4G4R4A4_UNORM;
                case PixelFormat::B5G6R5UNorm:      return DXGI_FORMAT_B5G6R5_UNORM;
                case PixelFormat::B5G5R5A1UNorm:    return DXGI_FORMAT_B5G5R5A1_UNORM;
                    // 32-bit formats
                case PixelFormat::R32UInt:          return DXGI_FORMAT_R32_UINT;
                case PixelFormat::R32SInt:          return DXGI_FORMAT_R32_SINT;
                case PixelFormat::R32Float:         return DXGI_FORMAT_R32_FLOAT;
                case PixelFormat::RG16UNorm:        return DXGI_FORMAT_R16G16_UNORM;
                case PixelFormat::RG16SNorm:        return DXGI_FORMAT_R16G16_SNORM;
                case PixelFormat::RG16UInt:         return DXGI_FORMAT_R16G16_UINT;
                case PixelFormat::RG16SInt:         return DXGI_FORMAT_R16G16_SINT;
                case PixelFormat::RG16Float:        return DXGI_FORMAT_R16G16_FLOAT;
                case PixelFormat::RGBA8UNorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
                case PixelFormat::RGBA8UNormSrgb:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case PixelFormat::RGBA8SNorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
                case PixelFormat::RGBA8UInt:        return DXGI_FORMAT_R8G8B8A8_UINT;
                case PixelFormat::RGBA8SInt:        return DXGI_FORMAT_R8G8B8A8_SINT;
                case PixelFormat::BGRA8UNorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
                case PixelFormat::BGRA8UNormSrgb:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                    // Packed 32-Bit formats
                case PixelFormat::RGB10A2UNorm:     return DXGI_FORMAT_R10G10B10A2_UNORM;
                case PixelFormat::RG11B10Float:     return DXGI_FORMAT_R11G11B10_FLOAT;
                case PixelFormat::RGB9E5Float:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                    // 64-Bit formats
                case PixelFormat::RG32UInt:         return DXGI_FORMAT_R32G32_UINT;
                case PixelFormat::RG32SInt:         return DXGI_FORMAT_R32G32_SINT;
                case PixelFormat::RG32Float:        return DXGI_FORMAT_R32G32_FLOAT;
                case PixelFormat::RGBA16UNorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
                case PixelFormat::RGBA16SNorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
                case PixelFormat::RGBA16UInt:       return DXGI_FORMAT_R16G16B16A16_UINT;
                case PixelFormat::RGBA16SInt:       return DXGI_FORMAT_R16G16B16A16_SINT;
                case PixelFormat::RGBA16Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
                    // 128-Bit formats
                case PixelFormat::RGBA32UInt:       return DXGI_FORMAT_R32G32B32A32_UINT;
                case PixelFormat::RGBA32SInt:       return DXGI_FORMAT_R32G32B32A32_SINT;
                case PixelFormat::RGBA32Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
                    // Depth-stencil formats
                case PixelFormat::Depth16UNorm:			return DXGI_FORMAT_D16_UNORM;
                case PixelFormat::Depth32Float:			return DXGI_FORMAT_D32_FLOAT;
                case PixelFormat::Depth24UNormStencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case PixelFormat::Depth32FloatStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                    // Compressed BC formats
                case PixelFormat::BC1UNorm:         return DXGI_FORMAT_BC1_UNORM;
                case PixelFormat::BC1UNormSrgb:     return DXGI_FORMAT_BC1_UNORM_SRGB;
                case PixelFormat::BC2UNorm:         return DXGI_FORMAT_BC2_UNORM;
                case PixelFormat::BC2UNormSrgb:     return DXGI_FORMAT_BC2_UNORM_SRGB;
                case PixelFormat::BC3UNorm:         return DXGI_FORMAT_BC3_UNORM;
                case PixelFormat::BC3UNormSrgb:     return DXGI_FORMAT_BC3_UNORM_SRGB;
                case PixelFormat::BC4SNorm:         return DXGI_FORMAT_BC4_SNORM;
                case PixelFormat::BC4UNorm:         return DXGI_FORMAT_BC4_UNORM;
                case PixelFormat::BC5SNorm:         return DXGI_FORMAT_BC5_SNORM;
                case PixelFormat::BC5UNorm:         return DXGI_FORMAT_BC5_UNORM;
                case PixelFormat::BC6HUFloat:       return DXGI_FORMAT_BC6H_UF16;
                case PixelFormat::BC6HSFloat:       return DXGI_FORMAT_BC6H_SF16;
                case PixelFormat::BC7UNorm:         return DXGI_FORMAT_BC7_UNORM;
                case PixelFormat::BC7UNormSrgb:     return DXGI_FORMAT_BC7_UNORM_SRGB;

                default:
                    ALIMER_UNREACHABLE();
                    return DXGI_FORMAT_UNKNOWN;
            }
        }

        [[nodiscard]] constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(PixelFormat format)
        {
            switch (format)
            {
                case PixelFormat::Depth16UNorm:
                    return DXGI_FORMAT_R16_TYPELESS;
                case PixelFormat::Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;
                case PixelFormat::Depth24UNormStencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case PixelFormat::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    ALIMER_ASSERT(IsDepthFormat(format) == false);
                    return ToDXGIFormat(format);
            }
        }

        [[nodiscard]] constexpr DXGI_FORMAT ToDXGIFormat(VertexFormat format)
        {
            switch (format)
            {
                case VertexFormat::UByte:           return DXGI_FORMAT_R8_UINT;
                case VertexFormat::UByte2:          return DXGI_FORMAT_R8G8_UINT;
                case VertexFormat::UByte4:          return DXGI_FORMAT_R8G8B8A8_UINT;
                case VertexFormat::Byte:            return DXGI_FORMAT_R8_SINT;
                case VertexFormat::Byte2:           return DXGI_FORMAT_R8G8_SINT;
                case VertexFormat::Byte4:           return DXGI_FORMAT_R8G8B8A8_SINT;
                case VertexFormat::UByteNorm:       return DXGI_FORMAT_R8_UNORM;
                case VertexFormat::UByte2Norm:      return DXGI_FORMAT_R8G8_UNORM;
                case VertexFormat::UByte4Norm:      return DXGI_FORMAT_R8G8B8A8_UNORM;
                case VertexFormat::ByteNorm:        return DXGI_FORMAT_R8_SNORM;
                case VertexFormat::Byte2Norm:       return DXGI_FORMAT_R8G8_SNORM;
                case VertexFormat::Byte4Norm:       return DXGI_FORMAT_R8G8B8A8_SNORM;

                case VertexFormat::UShort:          return DXGI_FORMAT_R16_UINT;
                case VertexFormat::UShort2:         return DXGI_FORMAT_R16G16_UINT;
                case VertexFormat::UShort4:         return DXGI_FORMAT_R16G16B16A16_UINT;
                case VertexFormat::Short:           return DXGI_FORMAT_R16_SINT;
                case VertexFormat::Short2:          return DXGI_FORMAT_R16G16_SINT;
                case VertexFormat::Short4:          return DXGI_FORMAT_R16G16B16A16_SINT;
                case VertexFormat::UShortNorm:      return DXGI_FORMAT_R16_UNORM;
                case VertexFormat::UShort2Norm:     return DXGI_FORMAT_R16G16_UNORM;
                case VertexFormat::UShort4Norm:     return DXGI_FORMAT_R16G16B16A16_UNORM;
                case VertexFormat::ShortNorm:       return DXGI_FORMAT_R16_SNORM;
                case VertexFormat::Short2Norm:      return DXGI_FORMAT_R16G16_SNORM;
                case VertexFormat::Short4Norm:      return DXGI_FORMAT_R16G16B16A16_SNORM;

                case VertexFormat::Half:            return DXGI_FORMAT_R16_FLOAT;
                case VertexFormat::Half2:           return DXGI_FORMAT_R16G16_FLOAT;
                case VertexFormat::Half4:           return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case VertexFormat::Float:           return DXGI_FORMAT_R32_FLOAT;
                case VertexFormat::Float2:          return DXGI_FORMAT_R32G32_FLOAT;
                case VertexFormat::Float3:          return DXGI_FORMAT_R32G32B32_FLOAT;
                case VertexFormat::Float4:          return DXGI_FORMAT_R32G32B32A32_FLOAT;

                case VertexFormat::UInt:            return DXGI_FORMAT_R32_UINT;
                case VertexFormat::UInt2:           return DXGI_FORMAT_R32G32_UINT;
                case VertexFormat::UInt3:           return DXGI_FORMAT_R32G32B32_UINT;
                case VertexFormat::UInt4:           return DXGI_FORMAT_R32G32B32A32_UINT;

                case VertexFormat::Int:             return DXGI_FORMAT_R32_SINT;
                case VertexFormat::Int2:            return DXGI_FORMAT_R32G32_SINT;
                case VertexFormat::Int3:            return DXGI_FORMAT_R32G32B32_SINT;
                case VertexFormat::Int4:            return DXGI_FORMAT_R32G32B32A32_SINT;

                case VertexFormat::RGB10A2Unorm:    return DXGI_FORMAT_R10G10B10A2_UNORM;

                default:
                    ALIMER_UNREACHABLE();
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

        [[nodiscard]] constexpr D3D12_FILTER_TYPE ToD3D12(SamplerFilter value)
        {
            switch (value)
            {
                case SamplerFilter::Point:
                    return D3D12_FILTER_TYPE_POINT;
                case SamplerFilter::Linear:
                    return D3D12_FILTER_TYPE_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_FILTER_TYPE_POINT;
            }
        }

        [[nodiscard]] constexpr D3D12_TEXTURE_ADDRESS_MODE ToD3D12(SamplerAddressMode mode)
        {
            switch (mode)
            {
                case SamplerAddressMode::Wrap:
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case SamplerAddressMode::Mirror:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case SamplerAddressMode::Clamp:
                    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case SamplerAddressMode::Border:
                    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                case SamplerAddressMode::MirrorOnce:
                    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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

        [[nodiscard]] constexpr D3D12_FILL_MODE ToD3D12(FillMode mode)
        {
            switch (mode)
            {
                default:
                case FillMode::Solid:
                    return D3D12_FILL_MODE_SOLID;
                case FillMode::Wireframe:
                    return D3D12_FILL_MODE_WIREFRAME;
            }
        }

        [[nodiscard]] constexpr D3D12_CULL_MODE ToD3D12(CullMode mode)
        {
            switch (mode)
            {
                default:
                case CullMode::None:
                    return D3D12_CULL_MODE_NONE;
                case CullMode::Front:
                    return D3D12_CULL_MODE_FRONT;
                case CullMode::Back:
                    return D3D12_CULL_MODE_BACK;
            }
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

        [[nodiscard]] constexpr VertexFormat GetFormatFromComponetType(D3D_REGISTER_COMPONENT_TYPE componentType, unsigned int componentsCount)
        {
            if (componentsCount == 1)
            {
                switch (componentType)
                {
                    case D3D_REGISTER_COMPONENT_UINT32:
                        return VertexFormat::UInt;
                    case D3D_REGISTER_COMPONENT_SINT32:
                        return VertexFormat::Int;
                    case D3D_REGISTER_COMPONENT_FLOAT32:
                        return VertexFormat::Float;
                    default:
                        break;
                }
            }
            else if (componentsCount <= 3)
            {
                switch (componentType)
                {
                    case D3D_REGISTER_COMPONENT_UINT32:
                        return VertexFormat::UInt2;
                    case D3D_REGISTER_COMPONENT_SINT32:
                        return VertexFormat::Int2;
                    case D3D_REGISTER_COMPONENT_FLOAT32:
                        return VertexFormat::Float2;
                    default:
                        break;
                }
            }
            else if (componentsCount <= 7)
            {
                switch (componentType)
                {
                    case D3D_REGISTER_COMPONENT_UINT32:
                        return VertexFormat::UInt3;
                    case D3D_REGISTER_COMPONENT_SINT32:
                        return VertexFormat::Int3;
                    case D3D_REGISTER_COMPONENT_FLOAT32:
                        return VertexFormat::Float3;
                    default:
                        break;
                }
            }
            else if (componentsCount <= 15)
            {
                switch (componentType)
                {
                    case D3D_REGISTER_COMPONENT_UINT32:
                        return VertexFormat::UInt4;
                    case D3D_REGISTER_COMPONENT_SINT32:
                        return VertexFormat::Int4;
                    case D3D_REGISTER_COMPONENT_FLOAT32:
                        return VertexFormat::Float4;
                    default:
                        break;
                }
            }

            // error, unknown dxgi format
            return VertexFormat::Undefined;
        }
    }

    extern IDxcUtils* dxcUtils;

    /* D3D12_Texture */
    D3D12_Texture::D3D12_Texture(const TextureDesc& desc)
        : Texture(desc)
    {
    }

    D3D12_Texture::~D3D12_Texture()
    {
        DestroyViews();
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

    std::unique_ptr<TextureView> D3D12_Texture::CreateView(const TextureViewDesc& desc)
    {
        return std::make_unique<D3D12TextureView>(this, desc);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12_Texture::GetRTV(uint32_t mipLevel, uint32_t slice, uint32_t arraySize)
    {
        if (arraySize == kAllArraySlices)
        {
            arraySize = GetArrayLayers() - slice;
        }
        else if (arraySize + slice > GetArrayLayers())
        {
            arraySize = GetArrayLayers() - slice;
        }

        D3D12_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), PixelFormat::Undefined, false);

        auto it = renderTargetViews.find(key);
        if (it == renderTargetViews.end())
        {
            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = ToDXGIFormat(key.format);

            switch (dimension)
            {
                case TextureDimension::Texture1D:
                    if (GetArrayLayers() > 1)
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice = mipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = slice;
                        viewDesc.Texture1DArray.ArraySize = arraySize;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = mipLevel;
                    }
                    break;
                case TextureDimension::Texture2D:
                    if (GetArrayLayers() > 1)
                    {
                        if (GetSampleCount() > 1)
                        {
                            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                            viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                            viewDesc.Texture2DMSArray.ArraySize = arraySize;
                        }
                        else
                        {
                            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                            viewDesc.Texture2DArray.MipSlice = mipLevel;
                            viewDesc.Texture2DArray.FirstArraySlice = slice;
                            viewDesc.Texture2DArray.ArraySize = arraySize;
                            viewDesc.Texture2DArray.PlaneSlice = 0;
                        }
                    }
                    else
                    {
                        if (GetSampleCount() > 1)
                        {
                            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                            viewDesc.Texture2D.MipSlice = mipLevel;
                        }
                    }
                    break;
                case TextureDimension::Texture3D:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                    viewDesc.Texture3D.MipSlice = mipLevel;
                    viewDesc.Texture3D.FirstWSlice = slice;
                    viewDesc.Texture3D.WSize = -1;
                    break;
                default:
                    LOGE("Texture has unsupported dimension for RTV");
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
            arraySize = GetArrayLayers() - slice;
        }
        else if (arraySize + slice > GetArrayLayers())
        {
            arraySize = GetArrayLayers() - slice;
        }

        D3D12_ViewKey key(TextureSubresourceSet(mipLevel, 1, slice, arraySize), PixelFormat::Undefined, isReadOnly);

        auto it = depthStencilViews.find(key);
        if (it == depthStencilViews.end())
        {
            //we haven't seen this one before
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
            viewDesc.Format = ToDXGIFormat(format);
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;

            if (isReadOnly)
            {
                viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
                if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
                    viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            }

            switch (dimension)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case TextureDimension::Texture1D:
                    if (GetArrayLayers() > 1)
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice = mipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = slice;
                        viewDesc.Texture1DArray.ArraySize = arraySize;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = mipLevel;
                    }
                    break;
                case TextureDimension::Texture2D:
                    if (GetArrayLayers() > 1)
                    {
                        if (GetSampleCount() > 1)
                        {
                            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                            viewDesc.Texture2DMSArray.FirstArraySlice = slice;
                            viewDesc.Texture2DMSArray.ArraySize = arraySize;
                        }
                        else
                        {
                            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                            viewDesc.Texture2DArray.MipSlice = mipLevel;
                            viewDesc.Texture2DArray.FirstArraySlice = slice;
                            viewDesc.Texture2DArray.ArraySize = arraySize;
                        }
                    }
                    else
                    {
                        if (GetSampleCount() > 1)
                        {
                            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                        }
                        else
                        {
                            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                            viewDesc.Texture2D.MipSlice = mipLevel;
                        }
                    }
                    break;
                default:
                    LOGE("Texture has unsupported dimension for DSV");
                    return {};
            }

            D3D12_CPU_DESCRIPTOR_HANDLE dsv = device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            device->GetD3DDevice()->CreateDepthStencilView(handle, &viewDesc, dsv);

            depthStencilViews[key] = dsv;
            return dsv;
        }

        return it->second;
    }

    /* D3D12TextureView */
    D3D12TextureView::D3D12TextureView(_In_ D3D12_Texture* texture, const TextureViewDesc& desc_)
        : TextureView(texture, desc_)
        , device(texture->device)
    {
        dxgiFormat = ToDXGIFormat(desc.format);

        const TextureUsage usage = texture->GetUsage();
        if (CheckBitsAny(usage, TextureUsage::RenderTarget))
        {
            if (!IsDepthStencilFormat(desc.format))
            {
                rtv = device->CreateRenderTargetView(texture->GetHandle(), desc);
            }
            else
            {
                //readWriteDSV = CreateDSV(texture->GetHandle(), false, false);
            }
        }
    }

    D3D12TextureView::~D3D12TextureView()
    {
    }

    /* D3D12_Buffer */
    D3D12_Buffer::D3D12_Buffer(const BufferDesc& desc)
        : Buffer(desc)
    {
    }

    D3D12_Buffer::~D3D12_Buffer()
    {
        if (handle != nullptr
            && allocation != nullptr)
        {
            device->DeferDestroy(handle, allocation);
        }

        handle = nullptr;
        allocation = nullptr;
    }

    /* D3D12_Sampler */
    D3D12_Sampler::D3D12_Sampler(const SamplerDesc& desc)
        : Sampler(desc)
    {
    }

    D3D12_Sampler::~D3D12_Sampler()
    {
        device->FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, descriptor);
        device->FreeBindlessSampler(bindlessIndex);
    }

    /* D3D12_Shader */
    D3D12_Shader::D3D12_Shader(ShaderStages stage, const void* bytecode_, size_t bytecodeLength)
        : Shader(stage)
    {
        bytecode.resize(bytecodeLength);
        memcpy(bytecode.data(), bytecode_, bytecodeLength);

        DxcBuffer ReflectionData;
        ReflectionData.Encoding = DXC_CP_ACP;
        ReflectionData.Ptr = bytecode.data();
        ReflectionData.Size = bytecode.size();

        RefCountPtr<ID3D12ShaderReflection> shaderReflection;
        ThrowIfFailed(dxcUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&shaderReflection)));

        D3D12_SHADER_DESC shaderDesc;
        ThrowIfFailed(shaderReflection->GetDesc(&shaderDesc));

        if (stage == ShaderStages::Vertex)
        {
            reflection.inputElements.reserve(shaderDesc.InputParameters);
            for (uint32_t i = 0; i < shaderDesc.InputParameters; ++i)
            {
                D3D12_SIGNATURE_PARAMETER_DESC parameterDesc;

                ThrowIfFailed(shaderReflection->GetInputParameterDesc(i, &parameterDesc));
                reflection.inputElements.push_back({ GetFormatFromComponetType(parameterDesc.ComponentType, parameterDesc.Mask), parameterDesc.SemanticName, parameterDesc.SemanticIndex });
            }
        }

        reflectionHash = 0;
        for (UINT i = 0; i < shaderDesc.BoundResources; i++)
        {
            D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
            ThrowIfFailed(shaderReflection->GetResourceBindingDesc(i, &bindingDesc));

            D3D12_SHADER_BUFFER_DESC bufferDesc = {};
            if (bindingDesc.Type == D3D_SIT_CBUFFER)
            {
                auto shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(i);
                ThrowIfFailed(shaderReflectionConstantBuffer->GetDesc(&bufferDesc));
            }

            ShaderReflection::ResourceBindType type = ShaderReflection::ResourceBindType::Unknown;
            switch (bindingDesc.Type)
            {
                case D3D_SIT_CBUFFER:
                    type = ShaderReflection::ResourceBindType::ConstantBuffer;
                    break;

                case D3D_SIT_TBUFFER:
                    type = ShaderReflection::ResourceBindType::ShaderResource;
                    break;

                case D3D_SIT_TEXTURE:
                    type = ShaderReflection::ResourceBindType::ShaderResource;
                    break;
                case D3D_SIT_SAMPLER:
                    type = ShaderReflection::ResourceBindType::Sampler;
                    break;
                case D3D_SIT_UAV_RWTYPED:
                    type = ShaderReflection::ResourceBindType::UnorderedAccess;
                    break;
                case D3D_SIT_STRUCTURED:
                    type = ShaderReflection::ResourceBindType::ShaderResource;
                    break;
                case D3D_SIT_UAV_RWSTRUCTURED:
                    type = ShaderReflection::ResourceBindType::UnorderedAccess;
                    break;
                case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
                    type = ShaderReflection::ResourceBindType::UnorderedAccess;
                    break;
                default:
                    ALIMER_UNREACHABLE();
                    break;
            }

            reflection.resources.push_back({ bindingDesc.Name, type, bindingDesc.Space, bindingDesc.BindPoint, bindingDesc.BindCount, bufferDesc.Size });

            HashCombine(reflectionHash, (u32)reflection.resources.back().type);
            HashCombine(reflectionHash, (u32)reflection.resources.back().set);
            HashCombine(reflectionHash, (u32)reflection.resources.back().binding);
            HashCombine(reflectionHash, (u32)reflection.resources.back().arraySize);
            HashCombine(reflectionHash, (u32)reflection.resources.back().size);
        }
    }

    /* D3D12_Pipeline */
    D3D12_Pipeline::D3D12_Pipeline(Type type)
        : Pipeline(type)
    {
    }

    D3D12_Pipeline::~D3D12_Pipeline()
    {
        if (rootSignature != nullptr)
        {
            device->DeferDestroy(rootSignature);
            rootSignature = nullptr;
        }

        device->DeferDestroy(handle);
        handle = nullptr;
    }

    inline const D3D12_Buffer* ToD3D12(const Buffer* resource)
    {
        return checked_cast<const D3D12_Buffer*>(resource);
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

        ID3D12DescriptorHeap* heaps[2] = {
            device->GetResourceDescriptorHeap(),
            device->GetSamplerDescriptorHeap()
        };
        handle->SetDescriptorHeaps(_countof(heaps), heaps);

        boundPipeline = nullptr;
        currentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        currentVbos = {};
        currentPass = {};
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
        passDesc.colorAttachmentCount = 1u;
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

        uint32_t numRenderTargets = 0;
        uint32_t width = UINT32_MAX;
        uint32_t height = UINT32_MAX;
        const D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES;

        for (uint32_t i = 0; i < desc.colorAttachmentCount; i++)
        {
            const RenderPassColorAttachment& attachment = desc.colorAttachments[i];
            ALIMER_ASSERT(attachment.texture);

            auto sourceTexture = checked_cast<D3D12_Texture*>(attachment.texture);

            const uint32_t mipLevel = attachment.mipLevel;
            const uint32_t slice = attachment.slice;

            width = Min(width, sourceTexture->GetWidth(mipLevel));
            height = Min(height, sourceTexture->GetHeight(mipLevel));

            rtvDescs[numRenderTargets].cpuDescriptor = sourceTexture->GetRTV(mipLevel, slice, 1);

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = sourceTexture->GetHandle();
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
                    rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction::Clear:
                    rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    rtvDescs[i].BeginningAccess.Clear.ClearValue.Format = sourceTexture->dxgiFormat;
                    rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor.r;
                    rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor.g;
                    rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor.b;
                    rtvDescs[i].BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor.a;
                    break;

                case LoadAction::Discard:
                    rtvDescs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.storeAction)
            {
                default:
                case StoreAction::Store:
                    rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    break;

                case StoreAction::Discard:
                    rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                    break;
            }

            if (attachment.resolveTexture != nullptr)
            {
                auto resolveTexture = checked_cast<D3D12_Texture*>(attachment.resolveTexture);

                //TransitionResource(sourceTexture, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, false);
                //TransitionResource(resolveTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST, false);
                //FlushResourceBarriers();

                rtvDescs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;
                rtvDescs[i].EndingAccess.Resolve.pSrcResource = sourceTexture->GetHandle();
                rtvDescs[i].EndingAccess.Resolve.pDstResource = resolveTexture->GetHandle();
                rtvDescs[i].EndingAccess.Resolve.SubresourceCount = 1;

                uint32_t dstSubresource = D3D12CalcSubresource(attachment.resolveLevel, attachment.resolveSlice, 0, resolveTexture->GetMipLevels(), resolveTexture->GetArrayLayers());
                uint32_t srcSubresource = D3D12CalcSubresource(attachment.mipLevel, attachment.slice, 0, sourceTexture->GetMipLevels(), sourceTexture->GetArrayLayers());

                subresourceParameters[i].SrcSubresource = srcSubresource;
                subresourceParameters[i].DstSubresource = dstSubresource;
                subresourceParameters[i].DstX = 0;
                subresourceParameters[i].DstY = 0;
                subresourceParameters[i].SrcRect = { 0, 0, 0, 0 };

                rtvDescs[i].EndingAccess.Resolve.pSubresourceParameters = &subresourceParameters[i];
                rtvDescs[i].EndingAccess.Resolve.Format = resolveTexture->dxgiFormat;

                // RESOLVE_MODE_AVERAGE is only valid for non-integer formats.
                switch (GetFormatKind(resolveTexture->GetFormat()))
                {
                    case PixelFormatKind::Integer:
                        rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_MAX;
                        break;
                    default:
                        rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
                        break;
                }

                rtvDescs[i].EndingAccess.Resolve.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;

                // Clear or preserve the resolve source.
                rtvDescs[i].EndingAccess.Resolve.PreserveResolveSource = attachment.storeAction == StoreAction::Store;
            }

            numRenderTargets++;
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsvDesc{};
        bool hasDepthStencil = false;
        if (desc.depthStencilAttachment.texture != nullptr)
        {
            hasDepthStencil = true;
            const RenderPassDepthStencilAttachment& attachment = desc.depthStencilAttachment;

            auto sourceTexture = checked_cast<D3D12_Texture*>(attachment.texture);

            width = Min(width, sourceTexture->GetWidth(attachment.mipLevel));
            height = Min(height, sourceTexture->GetHeight(attachment.mipLevel));

            dsvDesc.cpuDescriptor = sourceTexture->GetDSV(attachment.mipLevel, attachment.slice, 1, desc.depthStencilAttachment.depthStencilReadOnly);

            switch (attachment.depthLoadAction)
            {
                default:
                case LoadAction::Load:
                    dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction::Clear:
                    dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    dsvDesc.DepthBeginningAccess.Clear.ClearValue.Format = sourceTexture->dxgiFormat;
                    dsvDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = attachment.clearDepth;
                    break;

                case LoadAction::Discard:
                    dsvDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.depthStoreAction)
            {
                default:
                case StoreAction::Store:
                    dsvDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    break;

                case StoreAction::Discard:
                    dsvDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.stencilLoadAction)
            {
                default:
                case LoadAction::Load:
                    dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                    break;

                case LoadAction::Clear:
                    dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                    dsvDesc.StencilBeginningAccess.Clear.ClearValue.Format = sourceTexture->dxgiFormat;
                    dsvDesc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = attachment.clearStencil;
                    break;

                case LoadAction::Discard:
                    dsvDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                    break;
            }

            switch (attachment.stencilStoreAction)
            {
                default:
                case StoreAction::Store:
                    dsvDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                    break;

                case StoreAction::Discard:
                    dsvDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                    break;
            }
        }

        handle->BeginRenderPass(numRenderTargets, rtvDescs, hasDepthStencil ? &dsvDesc : nullptr, renderPassFlags);

        // The viewport and scissor default to cover all of the attachments
        const Viewport viewport(static_cast<float>(width), static_cast<float>(height));
        const D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
        const float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

        SetViewport(viewport);
        handle->RSSetScissorRects(1, &scissorRect);
        SetBlendColor(blendFactor);

        boundPipeline = nullptr;
    }

    void D3D12_CommandList::EndRenderPass()
    {
        handle->EndRenderPass();

        for (uint32_t i = 0; i < currentPass.colorAttachmentCount; i++)
        {
            const RenderPassColorAttachment& attachment = currentPass.colorAttachments[i];
            if (attachment.texture == nullptr)
                break;

            auto d3d11Texture = checked_cast<D3D12_Texture*>(attachment.texture);

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

        currentPass = {};
    }

    //void D3D12_CommandList::SetViewport(const Rect& rect)
    //{
    //    SetViewport(Viewport(rect));
    //}

    void D3D12_CommandList::SetViewport(const Viewport& viewport)
    {
        handle->RSSetViewports(1, (D3D12_VIEWPORT*)&viewport);
    }

    void D3D12_CommandList::SetViewports(const Viewport* viewports, uint32_t count)
    {
        handle->RSSetViewports(count, (D3D12_VIEWPORT*)viewports);
    }

    //void D3D12_CommandList::SetScissorRect(const Rect& rect)
    //{
    //    D3D12_RECT d3dScissorRect;
    //    d3dScissorRect.left = LONG(rect.x);
    //    d3dScissorRect.top = LONG(rect.y);
    //    d3dScissorRect.right = LONG(rect.x + rect.width);
    //    d3dScissorRect.bottom = LONG(rect.y + rect.height);
    //    handle->RSSetScissorRects(1, &d3dScissorRect);
    //}
    //
    //void D3D12_CommandList::SetScissorRects(const Rect* rects, uint32_t count)
    //{
    //    ALIMER_ASSERT(count <= kMaxViewportsAndScissors);
    //
    //    D3D12_RECT d3dScissorRects[kMaxViewportsAndScissors];
    //    for (uint32_t i = 0; i < count; i += 1)
    //    {
    //        d3dScissorRects[i].left = LONG(rects[i].x);
    //        d3dScissorRects[i].top = LONG(rects[i].y);
    //        d3dScissorRects[i].right = LONG(rects[i].x + rects[i].width);
    //        d3dScissorRects[i].bottom = LONG(rects[i].y + rects[i].height);
    //    }
    //    handle->RSSetScissorRects(count, d3dScissorRects);
    //}

    void D3D12_CommandList::SetStencilReference(uint32_t value)
    {
        handle->OMSetStencilRef(value);
    }

    void D3D12_CommandList::SetBlendColor(const Color& color)
    {
        handle->OMSetBlendFactor(color.data);
    }

    void D3D12_CommandList::SetBlendColor(const float blendColor[4])
    {
        handle->OMSetBlendFactor(blendColor);
    }

    void D3D12_CommandList::SetPipeline(_In_ Pipeline* pipeline)
    {
        boundPipeline = checked_cast<D3D12_Pipeline*>(pipeline);
        BindRenderPipeline();
    }

    void D3D12_CommandList::BindRenderPipeline()
    {
        handle->SetGraphicsRootSignature(boundPipeline->rootSignature);
        handle->SetPipelineState(boundPipeline->handle);

        if (currentPrimitiveTopology != boundPipeline->primitiveTopology)
        {
            currentPrimitiveTopology = boundPipeline->primitiveTopology;
            handle->IASetPrimitiveTopology(currentPrimitiveTopology);
        }
    }

    void D3D12_CommandList::SetVertexBuffer(uint32_t index, const Buffer* buffer)
    {
        ALIMER_ASSERT(index < kMaxVertexBufferBindings);

        const D3D12_Buffer* d3dBuffer = checked_cast<const D3D12_Buffer*>(buffer);
        uint64_t offset = 0;

        currentVbos[index].BufferLocation = d3dBuffer->deviceAddress + offset;
        currentVbos[index].SizeInBytes = (UINT)(buffer->GetSize() - offset);
    }

    void D3D12_CommandList::SetIndexBuffer(const Buffer* buffer, uint64_t offset, IndexType indexType)
    {
        const D3D12_Buffer* d3dBuffer = checked_cast<const D3D12_Buffer*>(buffer);
        const DXGI_FORMAT format = (indexType == IndexType::UInt32) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = d3dBuffer->deviceAddress + offset;
        view.SizeInBytes = (UINT)(d3dBuffer->GetSize() - offset);
        view.Format = format;
        handle->IASetIndexBuffer(&view);
    }

    void D3D12_CommandList::FlushDraw()
    {
        uint32_t startSlot = 0;
        uint32_t endSlot = 0;
        for (uint32_t slot = 0; slot < boundPipeline->vboSlotsUsed; ++slot)
        {
            startSlot = Min(startSlot, slot);
            endSlot = Max(endSlot, slot + 1);
            currentVbos[slot].StrideInBytes = boundPipeline->vboStrides[slot];
        }

        handle->IASetVertexBuffers(startSlot, endSlot, currentVbos.data());
    }

    void D3D12_CommandList::Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount, uint32_t baseInstance)
    {
        FlushDraw();

        handle->DrawInstanced(vertexCount, instanceCount, vertexStart, baseInstance);
    }

    void D3D12_CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t baseInstance)
    {
        FlushDraw();

        handle->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, baseInstance);
    }

    void D3D12_CommandList::BindConstantBufferCore(uint32_t binding, const Buffer* buffer, uint64_t offset, uint64_t range)
    {
        const D3D12_Buffer* d3dBuffer = checked_cast<const D3D12_Buffer*>(buffer);

        handle->SetGraphicsRootConstantBufferView(0, d3dBuffer->deviceAddress + offset);
    }

    /* D3D12CopyAllocator */
    D3D12CopyAllocator::D3D12CopyAllocator(D3D12_Device* device_)
        : device(device_)
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        ThrowIfFailed(device->GetD3DDevice()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)));
    }

    D3D12CopyAllocator::~D3D12CopyAllocator()
    {
        // Create fence and wait on it.
        RefCountPtr<ID3D12Fence> fence;
        ThrowIfFailed(device->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        ThrowIfFailed(queue->Signal(fence.Get(), 1));
        ThrowIfFailed(fence->SetEventOnCompletion(1, nullptr));
    }

    D3D12CopyAllocator::CopyCMD D3D12CopyAllocator::Allocate(u64 size)
    {
        locker.lock();

        // create a new command list if there are no free ones:
        if (freeList.empty())
        {
            CopyCMD cmd;

            ThrowIfFailed(
                device->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&cmd.commandAllocator))
            );

            ThrowIfFailed(
                device->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, cmd.commandAllocator.Get(), nullptr, IID_PPV_ARGS(&cmd.commandList))
            );

            ThrowIfFailed(cmd.commandList->Close());
            ThrowIfFailed(device->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&cmd.fence)));

            freeList.push_back(cmd);
        }

        CopyCMD cmd = freeList.back();
        if (cmd.uploadBuffer != nullptr &&
            cmd.uploadBuffer->GetSize() < size)
        {
            // Try to search for a staging buffer that can fit the request:
            for (size_t i = 0; i < freeList.size(); ++i)
            {
                if (freeList[i].uploadBuffer->GetSize() >= size)
                {
                    cmd = freeList[i];
                    std::swap(freeList[i], freeList.back());
                    break;
                }
            }
        }
        freeList.pop_back();
        locker.unlock();

        // If no buffer was found that fits the data, create one:
        if (cmd.uploadBuffer == nullptr ||
            cmd.uploadBuffer->GetSize() < size)
        {
            BufferDesc uploadBufferDesc;
            uploadBufferDesc.size = NextPowerOfTwo(size);
            uploadBufferDesc.heapType = HeapType::Upload;
            cmd.uploadBuffer = Buffer::Create(uploadBufferDesc, nullptr);
            ALIMER_ASSERT(cmd.uploadBuffer != nullptr);
        }

        // Reset and begin command list
        ThrowIfFailed(cmd.commandAllocator->Reset());
        ThrowIfFailed(cmd.commandList->Reset(cmd.commandAllocator.Get(), nullptr));

        return cmd;
    }

    void D3D12CopyAllocator::Submit(CopyCMD cmd)
    {
        HRESULT hr;

        cmd.commandList->Close();
        ID3D12CommandList* commandlists[] = {
            cmd.commandList.Get()
        };
        queue->ExecuteCommandLists(1, commandlists);
        hr = queue->Signal(cmd.fence.Get(), 1);
        assert(SUCCEEDED(hr));
        hr = cmd.fence->SetEventOnCompletion(1, nullptr);
        assert(SUCCEEDED(hr));
        hr = cmd.fence->Signal(0);
        assert(SUCCEEDED(hr));

        locker.lock();
        freeList.push_back(cmd);
        locker.unlock();
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
        WaitIdle();
        shuttingDown = true;

        copyAllocator.reset();

        // CPU Descriptor Heaps
        resourceDescriptorAllocator.Shutdown();
        samplerDescriptorAllocator.Shutdown();
        rtvDescriptorAllocator.Shutdown();
        dsvDescriptorAllocator.Shutdown();

        // Destroy create command lists (per queue)
        for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
        {
            for (u32 cmd = 0; cmd < kMaxCommandLists; cmd++)
            {
                commandLists[cmd][queue].reset();
            }
        }

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
            CD3DX12FeatureSupport d3dFeatures;
            ThrowIfFailed(d3dFeatures.Init(d3dDevice.Get()));
            featureLevel = d3dFeatures.MaxSupportedFeatureLevel();

            // Features
            features.rayTracing = d3dFeatures.RaytracingTier() >= D3D12_RAYTRACING_TIER_1_0;
            features.meshShader |= d3dFeatures.MeshShaderTier() >= D3D12_MESH_SHADER_TIER_1;

            // Limits
            limits.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
            limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            limits.maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
            limits.maxTextureArraySize = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            limits.minConstantBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
            limits.minStorageBufferOffsetAlignment = 16;
            limits.maxDrawIndirectCount = static_cast<uint32_t>(-1);

            /* see: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_format_support */
            UINT formatSupport = 0;
            for (u32 fmt = static_cast<u32>(PixelFormat::Undefined) + 1; fmt < static_cast<u32>(PixelFormat::Count); fmt++)
            {
                D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { ToDXGIFormat((PixelFormat)fmt), D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };

                if (formatSupport.Format == DXGI_FORMAT_UNKNOWN)
                    continue;

                HRESULT hr = d3dDevice->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
                if (FAILED(hr))
                    continue;
            }

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

        // Init copy allocator.
        copyAllocator = std::make_unique<D3D12CopyAllocator>(this);

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

        // Shader visible descriptor resource heap and bindless.
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            heapDesc.NumDescriptors = 1000000; // tier 1 limit
            ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&resourceHeap.handle)));

            resourceHeap.CPUStart = resourceHeap.handle->GetCPUDescriptorHandleForHeapStart();
            resourceHeap.GPUStart = resourceHeap.handle->GetGPUDescriptorHandleForHeapStart();

            ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&resourceHeap.fence)));
            resourceHeap.fenceValue = resourceHeap.fence->GetCompletedValue();

            for (u32 i = 0; i < kD3D12BindlessResourceCapacity; ++i)
            {
                freeBindlessResources.push_back(kD3D12BindlessResourceCapacity - i - 1);
            }
        }

        // Shader visible descriptor sampler heap and bindless.
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            heapDesc.NumDescriptors = 2048; // tier 1 limit
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&samplerHeap.handle)));
            samplerHeap.CPUStart = samplerHeap.handle->GetCPUDescriptorHandleForHeapStart();
            samplerHeap.GPUStart = samplerHeap.handle->GetGPUDescriptorHandleForHeapStart();

            ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&samplerHeap.fence)));
            samplerHeap.fenceValue = samplerHeap.fence->GetCompletedValue();

            for (uint32_t i = 0; i < kD3D12BindlessSamplerCapacity; ++i)
            {
                freeBindlessSamplers.push_back(kD3D12BindlessSamplerCapacity - i - 1);
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
        RefCountPtr<ID3D12Fence> fence;
        ThrowIfFailed(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        for (auto& queue : queues)
        {
            ThrowIfFailed(queue.queue->Signal(fence.Get(), 1));

            if (fence->GetCompletedValue() < 1)
            {
                ThrowIfFailed(fence->SetEventOnCompletion(1, nullptr));
            }
            fence->Signal(0);
        }
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

    CommandBuffer* D3D12_Device::BeginCommandBuffer(CommandQueue queue)
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
            queues[(uint8_t)submitQueue].submitCommandLists[queues[(uint8_t)submitQueue].submitCount++] = commandList;
        }

        // submit last cmd batch:
        ALIMER_ASSERT(submitQueue < CommandQueue::Count);
        ALIMER_ASSERT(queues[(uint8_t)submitQueue].submitCount > 0);
        queues[(uint8_t)submitQueue].queue->ExecuteCommandLists(
            queues[(uint8_t)submitQueue].submitCount,
            queues[(uint8_t)submitQueue].submitCommandLists
        );
        queues[(uint8_t)submitQueue].submitCount = 0;

        // Mark the completion of queues for this frame:
        for (uint8_t queue = 0; queue < (uint8_t)CommandQueue::Count; ++queue)
        {
            ThrowIfFailed(queues[queue].queue->Signal(queues[queue].frameFence[frameIndex].Get(), 1));
        }
    }

    bool D3D12_Device::BeginFrame()
    {
        if (deviceLost)
        {
            return false;
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

        return true;
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
                resourceHeap.fenceValue = resourceHeap.allocationOffset.load();
                ThrowIfFailed(GetGraphicsQueue()->Signal(resourceHeap.fence.Get(), resourceHeap.fenceValue));
                resourceHeap.cachedCompletedValue = resourceHeap.fence->GetCompletedValue();

                samplerHeap.fenceValue = samplerHeap.allocationOffset.load();
                ThrowIfFailed(GetGraphicsQueue()->Signal(samplerHeap.fence.Get(), samplerHeap.fenceValue));
                samplerHeap.cachedCompletedValue = samplerHeap.fence->GetCompletedValue();

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

        const TextureDesc backBufferTextureDesc = TextureDesc::Tex2D(PixelFormat::BGRA8UNorm,
            swapChainDesc.Width, swapChainDesc.Height, 1, 1, TextureUsage::RenderTarget
        );

        backBuffers.resize(swapChainDesc.BufferCount);
        for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
        {
            RefCountPtr<ID3D12Resource> d3d12BackBuffer;
            ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&d3d12BackBuffer)));

            backBuffers[i] = Texture::CreateExternal(d3d12BackBuffer.Get(), backBufferTextureDesc);
        }

        backBufferWidth = swapChainDesc.Width;
        backBufferHeight = swapChainDesc.Height;

        if (depthStencilFormat != PixelFormat::Undefined)
        {
            TextureDesc depthStencilTextureDesc = TextureDesc::Tex2D(depthStencilFormat, backBufferWidth, backBufferHeight, 1, 1, TextureUsage::RenderTarget);
            depthStencilTextureDesc.initialState = ResourceStates::DepthWrite;
            depthStencilTexture = Texture::Create(depthStencilTextureDesc);
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

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12_Device::CreateRenderTargetView(ID3D12Resource* resource, const TextureViewDesc& desc)
    {
        D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.Format = ToDXGIFormat(desc.format);

        // TODO

        D3D12_CPU_DESCRIPTOR_HANDLE rtv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        d3dDevice->CreateRenderTargetView(resource, &viewDesc, rtv);
        return rtv;
    }

    uint32_t D3D12_Device::AllocateBindlessResource(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        AcquireSRWLockExclusive(&destroyMutex);
        if (!freeBindlessResources.empty())
        {
            uint32_t index = freeBindlessResources.back();
            freeBindlessResources.pop_back();
            ReleaseSRWLockExclusive(&destroyMutex);

            D3D12_CPU_DESCRIPTOR_HANDLE dstBindless = resourceHeap.CPUStart;
            dstBindless.ptr += index * GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            d3dDevice->CopyDescriptorsSimple(1, dstBindless, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            return index;
        }

        ReleaseSRWLockExclusive(&destroyMutex);
        return {};
    }

    uint32_t D3D12_Device::AllocateBindlessSampler(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        AcquireSRWLockExclusive(&destroyMutex);
        if (!freeBindlessSamplers.empty())
        {
            uint32_t index = freeBindlessSamplers.back();
            freeBindlessSamplers.pop_back();
            ReleaseSRWLockExclusive(&destroyMutex);

            D3D12_CPU_DESCRIPTOR_HANDLE dstBindless = samplerHeap.CPUStart;
            dstBindless.ptr += index * GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            d3dDevice->CopyDescriptorsSimple(1, dstBindless, handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            return index;
        }

        ReleaseSRWLockExclusive(&destroyMutex);
        return {};
    }

    void D3D12_Device::FreeBindlessResource(uint32_t index)
    {
        if (index != kInvalidBindlessIndex)
        {
            AcquireSRWLockExclusive(&destroyMutex);
            destroyedBindlessResources.push_back(std::make_pair(index, frameCount));
            ReleaseSRWLockExclusive(&destroyMutex);
        }
    }

    void D3D12_Device::FreeBindlessSampler(uint32_t index)
    {
        if (index != kInvalidBindlessIndex)
        {
            AcquireSRWLockExclusive(&destroyMutex);
            destroyedBindlessSamplers.push_back(std::make_pair(index, frameCount));
            ReleaseSRWLockExclusive(&destroyMutex);
        }
    }

    Texture* D3D12_Device::GetCurrentBackBuffer() const
    {
        return backBuffers[swapChain->GetCurrentBackBufferIndex()].Get();
    }

    TextureRef D3D12_Device::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        auto texture = new D3D12_Texture(desc);
        texture->device = this;
        texture->dxgiFormat = ToDXGIFormat(desc.format);

        if (nativeHandle != nullptr)
        {
            texture->handle = (ID3D12Resource*)nativeHandle;
            return TextureRef::Create(texture);
        }

        DXGI_FORMAT format = texture->dxgiFormat;

        D3D12MA::ALLOCATION_DESC allocationDesc{};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Alignment = 0;
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(desc.depthOrArrayLayers);
        resourceDesc.MipLevels = static_cast<UINT16>(texture->GetMipLevels());
        resourceDesc.Format = ToDXGIFormat(desc.format);
        resourceDesc.SampleDesc.Count = desc.sampleCount;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        switch (desc.dimension)
        {
            case TextureDimension::Texture1D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;

            case TextureDimension::Texture2D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;

            case TextureDimension::Texture3D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                break;

            default:
                break;
        }

        D3D12_CLEAR_VALUE clearValue = {};
        D3D12_CLEAR_VALUE* pClearValue = nullptr;

        ResourceStates bestInitialState = ResourceStates::ShaderResource;
        if (CheckBitsAny(desc.usage, TextureUsage::RenderTarget))
        {
            // Render targets and Depth/Stencil targets are always committed resources
            allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;

            clearValue.Format = resourceDesc.Format;

            if (IsDepthStencilFormat(desc.format))
            {
                bestInitialState = ResourceStates::DepthWrite;
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                if (!CheckBitsAny(desc.usage, TextureUsage::ShaderRead))
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
            && CheckBitsAny(desc.usage, TextureUsage::ShaderRead))
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
            &texture->allocation,
            IID_PPV_ARGS(&texture->handle)
        );

        if (FAILED(hr))
        {
            delete texture;
            return nullptr;
        }


        //if (desc.label)
        //{
            //SetDebugName(handle, name.c_str());
        //}

        return TextureRef::Create(texture);
    }

    BufferRef D3D12_Device::CreateBuffer(const BufferDesc& desc, const void* initialData)
    {
        RefCountPtr<D3D12_Buffer> buffer = RefCountPtr<D3D12_Buffer>::Create(new D3D12_Buffer(desc));
        buffer->device = this;

        UINT64 size = desc.size;
        if ((desc.usage & BufferUsage::Constant) != 0)
        {
            size = AlignTo(desc.size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }

        D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        if (CheckBitsAll(desc.usage, BufferUsage::ShaderWrite))
        {
            resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        if (CheckBitsAny(desc.usage, BufferUsage::ShaderRead | BufferUsage::RayTracingAccelerationStructure))
        {
            resourceFlags &= ~D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size, resourceFlags);


        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

        if (desc.heapType == HeapType::Upload)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        else if (desc.heapType == HeapType::Readback)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        HRESULT hr = allocator->CreateResource(
            &allocationDesc,
            &resourceDesc,
            resourceState,
            nullptr,
            &buffer->allocation,
            IID_PPV_ARGS(&buffer->handle)
        );

        if (FAILED(hr))
        {
            return nullptr;
        }

        if (desc.label)
        {
            SetDebugName(buffer->handle, desc.label);
        }

        d3dDevice->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &buffer->allocatedSize);
        buffer->deviceAddress = buffer->handle->GetGPUVirtualAddress();

        if (desc.heapType == HeapType::Upload)
        {
            D3D12_RANGE readRange = {};
            ThrowIfFailed(buffer->handle->Map(0, &readRange, reinterpret_cast<void**>(&buffer->mappedData)));
        }
        else if (desc.heapType == HeapType::Readback)
        {
            ThrowIfFailed(buffer->handle->Map(0, nullptr, reinterpret_cast<void**>(&buffer->mappedData)));
        }

        // Copy data on request.
        if (initialData != nullptr)
        {
            auto cmd = copyAllocator->Allocate(desc.size);

            memcpy(cmd.uploadBuffer->MappedData(), initialData, desc.size);

            cmd.commandList->CopyBufferRegion(
                buffer->handle,
                0,
                ToD3D12(cmd.uploadBuffer.Get())->handle,
                0,
                desc.size
            );

            copyAllocator->Submit(cmd);
        }

        return buffer;
    }

    SamplerRef D3D12_Device::CreateSampler(const SamplerDesc& desc)
    {
        RefCountPtr<D3D12_Sampler> sampler = RefCountPtr<D3D12_Sampler>::Create(new D3D12_Sampler(desc));
        sampler->device = this;

        const D3D12_FILTER_TYPE minFilter = ToD3D12(desc.minFilter);
        const D3D12_FILTER_TYPE magFilter = ToD3D12(desc.magFilter);
        const D3D12_FILTER_TYPE mipFilter = ToD3D12(desc.mipmapFilter);

        D3D12_FILTER_REDUCTION_TYPE reduction = desc.compare == CompareFunction::Never
            ? D3D12_FILTER_REDUCTION_TYPE_STANDARD
            : D3D12_FILTER_REDUCTION_TYPE_COMPARISON;

        D3D12_SAMPLER_DESC samplerDesc{};

        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_sampler_desc
        if (desc.maxAnisotropy > 1)
        {
            samplerDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reduction);
        }
        else
        {
            samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, reduction);
        }

        samplerDesc.AddressU = ToD3D12(desc.addressModeU);
        samplerDesc.AddressV = ToD3D12(desc.addressModeV);
        samplerDesc.AddressW = ToD3D12(desc.addressModeW);
        samplerDesc.MipLODBias = desc.mipLodBias;
        samplerDesc.MaxAnisotropy = Min<UINT>(desc.maxAnisotropy, 16u);
        if (desc.compare != CompareFunction::Never)
        {
            samplerDesc.ComparisonFunc = ToD3D12(desc.compare);
        }
        else
        {
            samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        }

        switch (desc.borderColor)
        {
            case SamplerBorderColor::OpaqueBlack:
                samplerDesc.BorderColor[0] = 0.0f;
                samplerDesc.BorderColor[1] = 0.0f;
                samplerDesc.BorderColor[2] = 0.0f;
                samplerDesc.BorderColor[3] = 1.0f;
                break;

            case SamplerBorderColor::OpaqueWhite:
                samplerDesc.BorderColor[0] = 1.0f;
                samplerDesc.BorderColor[1] = 1.0f;
                samplerDesc.BorderColor[2] = 1.0f;
                samplerDesc.BorderColor[3] = 1.0f;
                break;
            default:
                samplerDesc.BorderColor[0] = 0.0f;
                samplerDesc.BorderColor[1] = 0.0f;
                samplerDesc.BorderColor[2] = 0.0f;
                samplerDesc.BorderColor[3] = 0.0f;
                break;
        }

        samplerDesc.MinLOD = desc.minLod;
        samplerDesc.MaxLOD = desc.maxLod;

        sampler->descriptor = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        d3dDevice->CreateSampler(&samplerDesc, sampler->descriptor);
        sampler->bindlessIndex = AllocateBindlessSampler(sampler->descriptor);

        return sampler;
    }

    ShaderRef D3D12_Device::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return ShaderRef::Create(new D3D12_Shader(stage, bytecode, bytecodeLength));
    }

    PipelineRef D3D12_Device::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        RefCountPtr<D3D12_Pipeline> pipeline = RefCountPtr<D3D12_Pipeline>::Create(new D3D12_Pipeline(Pipeline::Type::RenderPipeline));
        pipeline->device = this;

        std::vector<D3D12_DESCRIPTOR_RANGE1> descriptorRanges;
        std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

        const auto MatchShaderBindings = [&](Shader* shader)
        {
            if (!shader)
            {
                return;
            }

            for (const auto& resource : shader->GetReflection().resources)
            {
                if (resource.type == ShaderReflection::ResourceBindType::ConstantBuffer)
                {
                    pipeline->descriptorCBVParameterIndex = (uint32_t)rootParameters.size();
                    D3D12_ROOT_PARAMETER1& rootParameter = rootParameters.emplace_back();
                    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                    rootParameter.Descriptor.ShaderRegister = resource.binding;
                    rootParameter.Descriptor.RegisterSpace = resource.set;
                }
            }
        };

        MatchShaderBindings(desc.vertex);
        MatchShaderBindings(desc.pixel);

        D3D12_ROOT_SIGNATURE_DESC1 rootSigDesc = {};
        rootSigDesc.NumParameters = (UINT)rootParameters.size();
        rootSigDesc.pParameters = rootParameters.data();
        rootSigDesc.NumStaticSamplers = (UINT)staticSamplers.size();
        rootSigDesc.pStaticSamplers = staticSamplers.data();
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRSDesc = {};
        versionedRSDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versionedRSDesc.Desc_1_1 = rootSigDesc;

        RefCountPtr<ID3DBlob> blob;
        RefCountPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedRSDesc, blob.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            LOGE("Failed to create root signature: {}", (char*)errorBlob->GetBufferPointer());
            return nullptr;
        }

        ThrowIfFailed(
            d3dDevice->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&pipeline->rootSignature))
        );

        // Not found, create new one
        struct PSO_STREAM
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_VS VS;
            CD3DX12_PIPELINE_STATE_STREAM_HS HS;
            CD3DX12_PIPELINE_STATE_STREAM_DS DS;
            CD3DX12_PIPELINE_STATE_STREAM_GS GS;
            CD3DX12_PIPELINE_STATE_STREAM_PS PS;
            CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 DepthStencilState;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE IBStripCutValue;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
        } stream = {};

        stream.rootSignature = pipeline->rootSignature;

        D3D12_Shader* shader = checked_cast<D3D12_Shader*>(desc.vertex);
        stream.VS = { shader->bytecode.data(), shader->bytecode.size() };
        

        shader = checked_cast<D3D12_Shader*>(desc.hull);
        if (shader != nullptr)
        {
            stream.HS = { shader->bytecode.data(), shader->bytecode.size() };
        }

        shader = checked_cast<D3D12_Shader*>(desc.domain);
        if (shader != nullptr)
        {
            stream.DS = { shader->bytecode.data(), shader->bytecode.size() };
        }

        shader = checked_cast<D3D12_Shader*>(desc.geometry);
        if (shader != nullptr)
        {
            stream.GS = { shader->bytecode.data(), shader->bytecode.size() };
        }

        shader = checked_cast<D3D12_Shader*>(desc.pixel);
        if (shader != nullptr)
        {
            stream.PS = { shader->bytecode.data(), shader->bytecode.size() };
        }

        // Blend State and SampleMask
        CD3DX12_BLEND_DESC blendState = {};
        blendState.AlphaToCoverageEnable = desc.blendState.alphaToCoverageEnable;
        blendState.IndependentBlendEnable = desc.blendState.independentBlendEnable;
        for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
        {
            const RenderTargetBlendState& renderTarget = desc.blendState.renderTargets[i];
            blendState.RenderTarget[i].BlendEnable = renderTarget.blendEnable ? TRUE : FALSE;
            blendState.RenderTarget[i].SrcBlend = D3D12Blend(renderTarget.srcBlend);
            blendState.RenderTarget[i].DestBlend = D3D12Blend(renderTarget.destBlend);
            blendState.RenderTarget[i].BlendOp = D3D12BlendOperation(renderTarget.blendOp);
            blendState.RenderTarget[i].SrcBlendAlpha = D3D12AlphaBlend(renderTarget.srcBlendAlpha);
            blendState.RenderTarget[i].DestBlendAlpha = D3D12AlphaBlend(renderTarget.destBlendAlpha);
            blendState.RenderTarget[i].BlendOpAlpha = D3D12BlendOperation(renderTarget.blendOpAlpha);
            blendState.RenderTarget[i].RenderTargetWriteMask = D3D12RenderTargetWriteMask(renderTarget.writeMask);
            blendState.RenderTarget[i].LogicOpEnable = false;
            blendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
        }
        stream.BlendState = blendState;
        stream.SampleMask = UINT_MAX; // pipeline->GetSampleMask();

        CD3DX12_RASTERIZER_DESC rasterizerState = {};
        rasterizerState.FillMode = ToD3D12(desc.rasterizerState.fillMode);
        rasterizerState.CullMode = ToD3D12(desc.rasterizerState.cullMode);
        rasterizerState.FrontCounterClockwise = (desc.rasterizerState.frontFace == FaceWinding::CounterClockwise) ? TRUE : FALSE;
        rasterizerState.DepthBias = FloorToInt(desc.rasterizerState.depthBias * (float)(1 << 24));;
        rasterizerState.DepthBiasClamp = desc.rasterizerState.depthBiasClamp;
        rasterizerState.SlopeScaledDepthBias = desc.rasterizerState.depthBiasSlopeScale;
        rasterizerState.DepthClipEnable = TRUE;
        rasterizerState.MultisampleEnable = desc.sampleCount > 1 ? TRUE : FALSE;
        rasterizerState.AntialiasedLineEnable = FALSE;
        rasterizerState.ForcedSampleCount = 0;
        rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        stream.RasterizerState = rasterizerState;

        // DepthStencilState
        CD3DX12_DEPTH_STENCIL_DESC1 depthStencilState = {};
        depthStencilState.DepthEnable = (desc.depthStencilState.depthCompare != CompareFunction::Always || desc.depthStencilState.depthWriteEnable) ? TRUE : FALSE;
        depthStencilState.DepthWriteMask = desc.depthStencilState.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilState.DepthFunc = ToD3D12(desc.depthStencilState.depthCompare);
        depthStencilState.StencilEnable = StencilTestEnabled(&desc.depthStencilState) ? TRUE : FALSE;
        depthStencilState.StencilReadMask = desc.depthStencilState.stencilReadMask;
        depthStencilState.StencilWriteMask = desc.depthStencilState.stencilWriteMask;

        depthStencilState.FrontFace.StencilFailOp = ToD3D12(desc.depthStencilState.frontFace.failOp);
        depthStencilState.FrontFace.StencilDepthFailOp = ToD3D12(desc.depthStencilState.frontFace.depthFailOp);
        depthStencilState.FrontFace.StencilPassOp = ToD3D12(desc.depthStencilState.frontFace.passOp);
        depthStencilState.FrontFace.StencilFunc = ToD3D12(desc.depthStencilState.frontFace.compare);
        depthStencilState.BackFace.StencilFailOp = ToD3D12(desc.depthStencilState.backFace.failOp);
        depthStencilState.BackFace.StencilDepthFailOp = ToD3D12(desc.depthStencilState.backFace.depthFailOp);
        depthStencilState.BackFace.StencilPassOp = ToD3D12(desc.depthStencilState.backFace.passOp);
        depthStencilState.BackFace.StencilFunc = ToD3D12(desc.depthStencilState.backFace.compare);
        depthStencilState.DepthBoundsTestEnable = FALSE;

        stream.DepthStencilState = depthStencilState;

        // InputLayout
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        D3D12_INPUT_LAYOUT_DESC inputLayout = {};

        for (uint32_t location = 0; location < kMaxVertexAttributes; location++)
        {
            const VertexAttribute* attribute = &desc.vertexLayout.attributes[location];
            if (attribute->format == VertexFormat::Undefined)
                continue;

            auto& element = inputElements.emplace_back();;
            element.SemanticName = "ATTRIBUTE";
            element.SemanticIndex = location;
            element.Format = ToDXGIFormat(attribute->format);
            element.InputSlot = attribute->bufferIndex;
            element.AlignedByteOffset = attribute->offset;

            if (desc.vertexLayout.buffers[attribute->bufferIndex].stepRate == VertexStepRate::Vertex)
            {
                element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                element.InstanceDataStepRate = 0;
            }
            else
            {
                element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                element.InstanceDataStepRate = 1;
            }

            pipeline->vboSlotsUsed = Max(attribute->bufferIndex + 1, pipeline->vboSlotsUsed);
            pipeline->vboStrides[attribute->bufferIndex] = desc.vertexLayout.buffers[attribute->bufferIndex].stride;
        }

        inputLayout.pInputElementDescs = inputElements.data();
        inputLayout.NumElements = static_cast<UINT>(inputElements.size());
        stream.InputLayout = inputLayout;
        stream.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

        pipeline->primitiveTopology = ConvertPrimitiveTopology(desc.primitiveTopology, desc.patchControlPoints);
        switch (desc.primitiveTopology)
        {
            case PrimitiveTopology::PointList:
                stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                break;
            case PrimitiveTopology::LineList:
            case PrimitiveTopology::LineStrip:
                stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                break;
            case PrimitiveTopology::TriangleList:
            case PrimitiveTopology::TriangleStrip:
                stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                break;
            case PrimitiveTopology::PatchList:
                stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
                break;
            default:
                stream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                break;
        }

        D3D12_RT_FORMAT_ARRAY RTVFormats{};
        for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
        {
            if (desc.colorFormats[i] == PixelFormat::Undefined)
                continue;

            RTVFormats.RTFormats[RTVFormats.NumRenderTargets++] = ToDXGIFormat(desc.colorFormats[i]);
        }
        stream.RTVFormats = RTVFormats;
        stream.DSVFormat = ToDXGIFormat(desc.depthStencilFormat);

        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = desc.sampleCount;
        sampleDesc.Quality = 0;
        stream.SampleDesc = sampleDesc;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.pPipelineStateSubobjectStream = &stream;
        streamDesc.SizeInBytes = sizeof(stream);

        hr = d3dDevice->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipeline->handle));
        if (FAILED(hr))
        {
            LOGE("Failed to create RenderPipeline");
            return nullptr;
        }

        if (!desc.label.empty())
        {
            SetDebugName(pipeline->handle, desc.label);
        }

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

    bool InitializeD3D12Backend(Window* window, const PresentationParameters& presentationParameters)
    {
        if (!D3D12_Device::IsAvailable())
        {
            return false;
        }

        auto device = new D3D12_Device(presentationParameters.validationMode);
        gGraphics().Start(device);
        if (!device->Initialize(window, presentationParameters))
        {
            device = nullptr;
            gGraphics().Start(device);
            return false;
        }

        return gGraphics().IsInitialized();
    }
}

#endif /* defined(ALIMER_RHI_D3D12) */
