// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "AlimerConfig.h"

#if defined(ALIMER_RHI_D3D12)
#include "Core/Log.h"
#include "Core/BitOperations.h"
#include "Core/Vector.h"
#include "Core/UnorderedMap.h"
#include "Core/Hash.h"
#include "RHI/RHI.h"
#include "Platform/Win32/WindowsPlatform.h"

#ifdef _GAMING_XBOX_SCARLETT
#   pragma warning(push)
#   pragma warning(disable: 5204 5249)
#   include <d3d12_xs.h>
#   pragma warning(pop)
#   include <d3dx12_xs.h>
#elif (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
#   pragma warning(push)
#   pragma warning(disable: 5204)
#   include <d3d12_x.h>
#   pragma warning(pop)
#else
#   include <directx/d3d12.h>
#   include <directx/d3d12video.h>
#   include <directx/d3dx12_resource_helpers.h>
#   include <directx/d3dx12_pipeline_state_stream.h>
#   include <directx/d3dx12_check_feature_support.h>
#   include <dcomp.h>
#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//#   include "ThirdParty/microsoft.ui.xaml.media.dxinterop.h"
#else
//#   include <windows.ui.xaml.media.dxinterop.h> // WinRT
#endif
#   define PPV_ARGS(x) IID_PPV_ARGS(&x)
#endif

#include <wrl/client.h>

#include <dxgi1_6.h>

#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#   include <dxgidebug.h>
#endif

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemAlloc.h"

//#define ALIMER_USE_PIX3

#if defined(ALIMER_USE_PIX3)
#include <pix3.h>
#else
// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix.h>
#endif

#include <array>
#include <mutex>
#include <deque>
#include <memory>
#include <sstream>

using Microsoft::WRL::ComPtr;

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace D3D12MA
{
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(ALLOCATOR_FLAGS);
}

namespace Alimer
{
    namespace
    {
        void WINAPI PIXBeginEventOnCommandListFn(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
        void WINAPI PIXEndEventOnCommandListFn(ID3D12GraphicsCommandList* commandList);
        void WINAPI PIXSetMarkerOnCommandListFn(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
        //void WINAPI PIXBeginEventOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);
        //void WINAPI PIXEndEventOnCommandQueue(ID3D12CommandQueue* commandQueue);
        //void WINAPI PIXSetMarkerOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);

        static_assert(sizeof(GPUAddress) == sizeof(D3D12_GPU_VIRTUAL_ADDRESS), "GPUAddress mismatch");

        static_assert(sizeof(RHIViewport) == sizeof(D3D12_VIEWPORT), "RHIViewport mismatch");
        static_assert(offsetof(RHIViewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "RHIViewport Layout mismatch");
        static_assert(offsetof(RHIViewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "RHIViewport Layout mismatch");
        static_assert(offsetof(RHIViewport, width) == offsetof(D3D12_VIEWPORT, Width), "RHIViewport Layout mismatch");
        static_assert(offsetof(RHIViewport, height) == offsetof(D3D12_VIEWPORT, Height), "RHIViewport Layout mismatch");
        static_assert(offsetof(RHIViewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "RHIViewport Layout mismatch");
        static_assert(offsetof(RHIViewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "RHIViewport Layout mismatch");

        static_assert(sizeof(DispatchIndirectCommand) == sizeof(D3D12_DISPATCH_ARGUMENTS), "DispatchIndirectCommand mismatch");
        static_assert(offsetof(DispatchIndirectCommand, x) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountX), "Layout mismatch");
        static_assert(offsetof(DispatchIndirectCommand, y) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountY), "Layout mismatch");
        static_assert(offsetof(DispatchIndirectCommand, z) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountZ), "Layout mismatch");

        static_assert(sizeof(DrawIndirectCommand) == sizeof(D3D12_DRAW_ARGUMENTS), "DrawIndirectCommand mismatch");
        static_assert(offsetof(DrawIndirectCommand, vertexCount) == offsetof(D3D12_DRAW_ARGUMENTS, VertexCountPerInstance), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, instanceCount) == offsetof(D3D12_DRAW_ARGUMENTS, InstanceCount), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, firstVertex) == offsetof(D3D12_DRAW_ARGUMENTS, StartVertexLocation), "Layout mismatch");
        static_assert(offsetof(DrawIndirectCommand, firstInstance) == offsetof(D3D12_DRAW_ARGUMENTS, StartInstanceLocation), "Layout mismatch");

        static_assert(sizeof(DrawIndexedIndirectCommand) == sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), "DrawIndexedIndirectCommand mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, indexCount) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, IndexCountPerInstance), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, instanceCount) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, InstanceCount), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, firstIndex) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, StartIndexLocation), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, baseVertex) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, BaseVertexLocation), "Layout mismatch");
        static_assert(offsetof(DrawIndexedIndirectCommand, firstInstance) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, StartInstanceLocation), "Layout mismatch");

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
        static PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

        static PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
        static PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
        static PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

        static constexpr IID CLSID_D3D12Debug_Alimer = { 0xf2352aeb, 0xdd84, 0x49fe, {0xb9, 0x7b, 0xa9, 0xdc, 0xfd, 0xcc, 0x1b, 0x4f} };
        static constexpr IID CLSID_D3D12DeviceRemovedExtendedData_Alimer = { 0x4a75bbc4, 0x9ff4, 0x4ad8, {0x9f, 0x18, 0xab, 0xae, 0x84, 0xdc, 0x5f, 0xf2} };
        static constexpr IID CLSID_D3D12SDKConfiguration_Alimer = { 0x7cda6aca, 0xa03e, 0x49c8, {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce} };
        static constexpr IID CLSID_D3D12DeviceFactory_Alimer = { 0x114863bf, 0xc386, 0x4aee, {0xb3, 0x9d, 0x8f, 0x0b, 0xbb, 0x06, 0x29, 0x55} };
#if defined(_DEBUG)
        using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);
        static PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;

        // Declare debug guids to avoid linking with "dxguid.lib"
        static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
        static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };
#endif

        static constexpr IID ID_D3DDebugObjectName = { 0x429b8c22, 0x9188, 0x4b0c, {0x87,0x42,0xac,0xb0,0xbf,0x85,0xc2,0x00} };

        using PFN_PIXBeginEventOnCommandList = decltype(&PIXBeginEventOnCommandListFn);
        using PFN_PIXEndEventOnCommandList = decltype(&PIXEndEventOnCommandListFn);
        using PFN_PIXSetMarkerOnCommandList = decltype(&PIXSetMarkerOnCommandListFn);

        static PFN_PIXBeginEventOnCommandList PIXBeginEventOnCommandList = nullptr;
        static PFN_PIXEndEventOnCommandList PIXEndEventOnCommandList = nullptr;
        static PFN_PIXSetMarkerOnCommandList PIXSetMarkerOnCommandList = nullptr;
#endif

        inline DXGI_FORMAT ToDxgiRTVFormat(PixelFormat format)
        {
            return static_cast<DXGI_FORMAT>(ToDxgiFormat(format));
        }

        inline DXGI_FORMAT ToDxgiDSVFormat(PixelFormat format)
        {
            return static_cast<DXGI_FORMAT>(ToDxgiFormat(format));
        }

        inline DXGI_FORMAT ToDxgiSRVFormat(PixelFormat format)
        {
            // Try to resolve resource format:
            switch (format)
            {
                case PixelFormat::Depth16Unorm:
                    return DXGI_FORMAT_R16_UNORM;

                case PixelFormat::Depth32Float:
                    return DXGI_FORMAT_R32_FLOAT;

                case PixelFormat::Stencil8:
                case PixelFormat::Depth24UnormStencil8:
                    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

                case PixelFormat::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

                    //case PixelFormat::NV12:
                    //    srvDesc.Format = DXGI_FORMAT_R8_UNORM;
                    //    break;
                default:
                    return static_cast<DXGI_FORMAT>(ToDxgiFormat(format));
            }
        }

        inline DXGI_FORMAT ToDxgiUAVFormat(PixelFormat format)
        {
            return static_cast<DXGI_FORMAT>(ToDxgiFormat(format));
        }

        constexpr DXGI_FORMAT ToDxgiFormat(RHIVertexAttributeFormat format)
        {
            switch (format)
            {
                case RHIVertexAttributeFormat::Uint8:               return DXGI_FORMAT_R8_UINT;
                case RHIVertexAttributeFormat::Uint8x2:             return DXGI_FORMAT_R8G8_UINT;
                case RHIVertexAttributeFormat::Uint8x4:             return DXGI_FORMAT_R8G8B8A8_UINT;
                case RHIVertexAttributeFormat::Sint8:               return DXGI_FORMAT_R8_SINT;
                case RHIVertexAttributeFormat::Sint8x2:             return DXGI_FORMAT_R8G8_SINT;
                case RHIVertexAttributeFormat::Sint8x4:             return DXGI_FORMAT_R8G8B8A8_SINT;
                case RHIVertexAttributeFormat::Unorm8:              return DXGI_FORMAT_R8_UNORM;
                case RHIVertexAttributeFormat::Unorm8x2:            return DXGI_FORMAT_R8G8_UNORM;
                case RHIVertexAttributeFormat::Unorm8x4:            return DXGI_FORMAT_R8G8B8A8_UNORM;
                case RHIVertexAttributeFormat::Snorm8:              return DXGI_FORMAT_R8_SNORM;
                case RHIVertexAttributeFormat::Snorm8x2:            return DXGI_FORMAT_R8G8_SNORM;
                case RHIVertexAttributeFormat::Snorm8x4:            return DXGI_FORMAT_R8G8B8A8_SNORM;

                case RHIVertexAttributeFormat::Uint16:              return DXGI_FORMAT_R16_UINT;
                case RHIVertexAttributeFormat::Uint16x2:            return DXGI_FORMAT_R16G16_UINT;
                case RHIVertexAttributeFormat::Uint16x4:            return DXGI_FORMAT_R16G16B16A16_UINT;
                case RHIVertexAttributeFormat::Sint16:              return DXGI_FORMAT_R16_SINT;
                case RHIVertexAttributeFormat::Sint16x2:            return DXGI_FORMAT_R16G16_SINT;
                case RHIVertexAttributeFormat::Sint16x4:            return DXGI_FORMAT_R16G16B16A16_SINT;
                case RHIVertexAttributeFormat::Unorm16:             return DXGI_FORMAT_R16_UNORM;
                case RHIVertexAttributeFormat::Unorm16x2:           return DXGI_FORMAT_R16G16_UNORM;
                case RHIVertexAttributeFormat::Unorm16x4:           return DXGI_FORMAT_R16G16B16A16_UNORM;
                case RHIVertexAttributeFormat::Snorm16:             return DXGI_FORMAT_R16_SNORM;
                case RHIVertexAttributeFormat::Snorm16x2:           return DXGI_FORMAT_R16G16_SNORM;
                case RHIVertexAttributeFormat::Snorm16x4:           return DXGI_FORMAT_R16G16B16A16_SNORM;
                case RHIVertexAttributeFormat::Float16:             return DXGI_FORMAT_R16_FLOAT;
                case RHIVertexAttributeFormat::Float16x2:           return DXGI_FORMAT_R16G16_FLOAT;
                case RHIVertexAttributeFormat::Float16x4:           return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case RHIVertexAttributeFormat::Float32:             return DXGI_FORMAT_R32_FLOAT;
                case RHIVertexAttributeFormat::Float32x2:           return DXGI_FORMAT_R32G32_FLOAT;
                case RHIVertexAttributeFormat::Float32x3:           return DXGI_FORMAT_R32G32B32_FLOAT;
                case RHIVertexAttributeFormat::Float32x4:           return DXGI_FORMAT_R32G32B32A32_FLOAT;

                case RHIVertexAttributeFormat::Uint32:              return DXGI_FORMAT_R32_UINT;
                case RHIVertexAttributeFormat::Uint32x2:            return DXGI_FORMAT_R32G32_UINT;
                case RHIVertexAttributeFormat::Uint32x3:            return DXGI_FORMAT_R32G32B32_UINT;
                case RHIVertexAttributeFormat::Uint32x4:            return DXGI_FORMAT_R32G32B32A32_UINT;

                case RHIVertexAttributeFormat::Sint32:              return DXGI_FORMAT_R32_SINT;
                case RHIVertexAttributeFormat::Sint32x2:            return DXGI_FORMAT_R32G32_SINT;
                case RHIVertexAttributeFormat::Sint32x3:            return DXGI_FORMAT_R32G32B32_SINT;
                case RHIVertexAttributeFormat::Sint32x4:            return DXGI_FORMAT_R32G32B32A32_SINT;

                case RHIVertexAttributeFormat::Unorm10_10_10_2:   return DXGI_FORMAT_R10G10B10A2_UNORM;
                case RHIVertexAttributeFormat::Unorm8x4BGRA:   return DXGI_FORMAT_B8G8R8A8_UNORM;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(PixelFormat format)
        {
            bool UsePackedDepth24UnormStencil8Format = true;

            switch (format)
            {
                case PixelFormat::Stencil8:
                    return DXGI_FORMAT_R24G8_TYPELESS;
                case PixelFormat::Depth16Unorm:
                    return DXGI_FORMAT_R16_TYPELESS;

                case PixelFormat::Depth32Float:
                    return DXGI_FORMAT_R32_TYPELESS;

                case PixelFormat::Depth24UnormStencil8:
                    return UsePackedDepth24UnormStencil8Format ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32G8X24_TYPELESS;
                case PixelFormat::Depth32FloatStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                default:
                    ALIMER_ASSERT(IsDepthFormat(format) == false);
                    return (DXGI_FORMAT)ToDxgiFormat(format);
            }
        }

        constexpr D3D12_COMMAND_LIST_TYPE ToD3D12(RHIQueueType type)
        {
            switch (type)
            {
                case RHIQueueType::Graphics:            return D3D12_COMMAND_LIST_TYPE_DIRECT;
                case RHIQueueType::Compute:             return D3D12_COMMAND_LIST_TYPE_COMPUTE;
                case RHIQueueType::Copy:                return D3D12_COMMAND_LIST_TYPE_COPY;
                case RHIQueueType::VideoDecode:         return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr D3D12_COMPARISON_FUNC ToD3D12(RHICompareFunction function)
        {
            switch (function)
            {
                case RHICompareFunction::Never:        return D3D12_COMPARISON_FUNC_NEVER;
                case RHICompareFunction::Less:         return D3D12_COMPARISON_FUNC_LESS;
                case RHICompareFunction::Equal:        return D3D12_COMPARISON_FUNC_EQUAL;
                case RHICompareFunction::LessEqual:    return D3D12_COMPARISON_FUNC_LESS_EQUAL;
                case RHICompareFunction::Greater:      return D3D12_COMPARISON_FUNC_GREATER;
                case RHICompareFunction::NotEqual:     return D3D12_COMPARISON_FUNC_NOT_EQUAL;
                case RHICompareFunction::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
                case RHICompareFunction::Always:       return D3D12_COMPARISON_FUNC_ALWAYS;

                default:
                    return static_cast<D3D12_COMPARISON_FUNC>(0);
            }
        }

        constexpr D3D12_STENCIL_OP ToD3D12(RHIStencilOperation op)
        {
            switch (op)
            {
                case RHIStencilOperation::Keep:            return D3D12_STENCIL_OP_KEEP;
                case RHIStencilOperation::Zero:            return D3D12_STENCIL_OP_ZERO;
                case RHIStencilOperation::Replace:         return D3D12_STENCIL_OP_REPLACE;
                case RHIStencilOperation::Invert:          return D3D12_STENCIL_OP_INVERT;
                case RHIStencilOperation::IncrementClamp:  return D3D12_STENCIL_OP_INCR_SAT;
                case RHIStencilOperation::DecrementClamp:  return D3D12_STENCIL_OP_DECR_SAT;
                case RHIStencilOperation::IncrementWrap:   return D3D12_STENCIL_OP_INCR;
                case RHIStencilOperation::DecrementWrap:   return D3D12_STENCIL_OP_DECR;
                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr D3D12_INPUT_CLASSIFICATION ToD3D12(RHIVertexStepMode mode)
        {
            switch (mode)
            {
                case RHIVertexStepMode::Vertex:     return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                case RHIVertexStepMode::Instance:   return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                default:                            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            }
        }

        constexpr D3D12_FILTER_TYPE ToD3D12(RHISamplerMinMagFilter filter)
        {
            switch (filter)
            {
                case RHISamplerMinMagFilter::Point:     return D3D12_FILTER_TYPE_POINT;
                case RHISamplerMinMagFilter::Linear:    return D3D12_FILTER_TYPE_LINEAR;
                default:                                return D3D12_FILTER_TYPE_POINT;
            }
        }

        constexpr D3D12_FILTER_TYPE ToD3D12(RHISamplerMipFilter filter)
        {
            switch (filter)
            {
                case RHISamplerMipFilter::Point:
                    return D3D12_FILTER_TYPE_POINT;
                case RHISamplerMipFilter::Linear:
                    return D3D12_FILTER_TYPE_LINEAR;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_FILTER_TYPE_POINT;
            }
        }

        constexpr D3D12_FILTER_REDUCTION_TYPE ToD3D12(RHISamplerReductionType reductionType)
        {
            switch (reductionType)
            {
                case RHISamplerReductionType::Standard:     return D3D12_FILTER_REDUCTION_TYPE_STANDARD;
                case RHISamplerReductionType::Comparison:   return D3D12_FILTER_REDUCTION_TYPE_COMPARISON;
                case RHISamplerReductionType::Minimum:      return D3D12_FILTER_REDUCTION_TYPE_MINIMUM;
                case RHISamplerReductionType::Maximum:      return D3D12_FILTER_REDUCTION_TYPE_MAXIMUM;
                default:                                    return D3D12_FILTER_REDUCTION_TYPE_STANDARD;
            }
        }

        constexpr D3D12_TEXTURE_ADDRESS_MODE ToD3D12(RHISamplerAddressMode mode)
        {
            switch (mode)
            {
                case RHISamplerAddressMode::Clamp:         return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                case RHISamplerAddressMode::Wrap:          return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
                case RHISamplerAddressMode::Mirror:        return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
                case RHISamplerAddressMode::MirrorOnce:    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
                case RHISamplerAddressMode::Border:        return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                default:
                    ALIMER_UNREACHABLE();
                    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            }
        }

        constexpr D3D12_BLEND D3D12Blend(RHIBlendFactor factor)
        {
            switch (factor)
            {
                case RHIBlendFactor::Zero:                      return D3D12_BLEND_ZERO;
                case RHIBlendFactor::One:                       return D3D12_BLEND_ONE;
                case RHIBlendFactor::SourceColor:               return D3D12_BLEND_SRC_COLOR;
                case RHIBlendFactor::OneMinusSourceColor:       return D3D12_BLEND_INV_SRC_COLOR;
                case RHIBlendFactor::SourceAlpha:               return D3D12_BLEND_SRC_ALPHA;
                case RHIBlendFactor::OneMinusSourceAlpha:       return D3D12_BLEND_INV_SRC_ALPHA;
                case RHIBlendFactor::DestinationColor:          return D3D12_BLEND_DEST_COLOR;
                case RHIBlendFactor::OneMinusDestinationColor:  return D3D12_BLEND_INV_DEST_COLOR;
                case RHIBlendFactor::DestinationAlpha:          return D3D12_BLEND_DEST_ALPHA;
                case RHIBlendFactor::OneMinusDestinationAlpha:  return D3D12_BLEND_INV_DEST_ALPHA;
                case RHIBlendFactor::SourceAlphaSaturate:       return D3D12_BLEND_SRC_ALPHA_SAT;
                case RHIBlendFactor::BlendColor:                return D3D12_BLEND_BLEND_FACTOR;
                case RHIBlendFactor::OneMinusBlendColor:        return D3D12_BLEND_INV_BLEND_FACTOR;
                case RHIBlendFactor::BlendAlpha:                return D3D12_BLEND_ALPHA_FACTOR;
                case RHIBlendFactor::OneMinusBlendAlpha:        return D3D12_BLEND_INV_ALPHA_FACTOR;
                case RHIBlendFactor::Source1Color:              return D3D12_BLEND_SRC1_COLOR;
                case RHIBlendFactor::OneMinusSource1Color:      return D3D12_BLEND_INV_SRC1_COLOR;
                case RHIBlendFactor::Source1Alpha:              return D3D12_BLEND_SRC1_ALPHA;
                case RHIBlendFactor::OneMinusSource1Alpha:      return D3D12_BLEND_INV_SRC1_ALPHA;
                default:
                    return D3D12_BLEND_ZERO;
            }
        }

        constexpr D3D12_BLEND D3D12AlphaBlend(RHIBlendFactor factor)
        {
            switch (factor)
            {
                case RHIBlendFactor::SourceColor:               return D3D12_BLEND_SRC_ALPHA;
                case RHIBlendFactor::OneMinusSourceColor:       return D3D12_BLEND_INV_SRC_ALPHA;
                case RHIBlendFactor::DestinationColor:          return D3D12_BLEND_DEST_ALPHA;
                case RHIBlendFactor::OneMinusDestinationColor:  return D3D12_BLEND_INV_DEST_ALPHA;
                case RHIBlendFactor::Source1Color:              return D3D12_BLEND_SRC1_ALPHA;
                case RHIBlendFactor::OneMinusSource1Color:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                    // Other blend factors translate to the same D3D12 enum as the color blend factors.
                default:
                    return D3D12Blend(factor);
            }
        }

        constexpr D3D12_BLEND_OP D3D12BlendOperation(RHIBlendOperation operation)
        {
            switch (operation)
            {
                case RHIBlendOperation::Add:                return D3D12_BLEND_OP_ADD;
                case RHIBlendOperation::Subtract:           return D3D12_BLEND_OP_SUBTRACT;
                case RHIBlendOperation::ReverseSubtract:    return D3D12_BLEND_OP_REV_SUBTRACT;
                case RHIBlendOperation::Min:                return D3D12_BLEND_OP_MIN;
                case RHIBlendOperation::Max:                return D3D12_BLEND_OP_MAX;
                default:                                    return D3D12_BLEND_OP_ADD;
            }
        }

        constexpr uint8_t ToD3D12(RHIColorWriteMask writeMask)
        {
            static_assert(static_cast<UINT>(RHIColorWriteMask::Red) == D3D12_COLOR_WRITE_ENABLE_RED);
            static_assert(static_cast<UINT>(RHIColorWriteMask::Green) == D3D12_COLOR_WRITE_ENABLE_GREEN);
            static_assert(static_cast<UINT>(RHIColorWriteMask::Blue) == D3D12_COLOR_WRITE_ENABLE_BLUE);
            static_assert(static_cast<UINT>(RHIColorWriteMask::Alpha) == D3D12_COLOR_WRITE_ENABLE_ALPHA);

            return static_cast<uint8_t>(writeMask);
        }

        constexpr D3D12_FILL_MODE ToD3D12(RHIFillMode mode)
        {
            switch (mode)
            {
                case RHIFillMode::Solid:        return D3D12_FILL_MODE_SOLID;
                case RHIFillMode::Wireframe:    return D3D12_FILL_MODE_WIREFRAME;
                default:                        return D3D12_FILL_MODE_WIREFRAME;
            }
        }

        constexpr D3D12_CULL_MODE ToD3D12(RHICullMode mode)
        {
            switch (mode)
            {
                case RHICullMode::Back:     return D3D12_CULL_MODE_BACK;
                case RHICullMode::None:     return D3D12_CULL_MODE_NONE;
                case RHICullMode::Front:    return D3D12_CULL_MODE_FRONT;

                default:                    return D3D12_CULL_MODE_BACK;
            }
        }

        constexpr D3D12_SHADER_VISIBILITY ToD3D12(RHIShaderStages stage)
        {
            switch (stage)  // NOLINT(clang-diagnostic-switch-enum)
            {
                case RHIShaderStages::Vertex:
                    return D3D12_SHADER_VISIBILITY_VERTEX;
                case RHIShaderStages::Fragment:
                    return D3D12_SHADER_VISIBILITY_PIXEL;
                case RHIShaderStages::Amplification:
                    return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
                case RHIShaderStages::Mesh:
                    return D3D12_SHADER_VISIBILITY_MESH;

                default:
                    // catch-all case - actually some of the bitfield combinations are unrepresentable in DX12
                    return D3D12_SHADER_VISIBILITY_ALL;
            }
        }

        D3D12_DEPTH_STENCILOP_DESC ToD3D12StencilOpDesc(const RHIStencilFaceState& state)
        {
            D3D12_DEPTH_STENCILOP_DESC desc = {};
            desc.StencilFailOp = ToD3D12(state.failOp);
            desc.StencilDepthFailOp = ToD3D12(state.depthFailOp);
            desc.StencilPassOp = ToD3D12(state.passOp);
            desc.StencilFunc = ToD3D12(state.compareFunc);
            return desc;
        }

        D3D12_SAMPLER_DESC ToD3D12SamplerDesc(const RHISamplerDesc& desc)
        {
            const D3D12_FILTER_TYPE minFilter = ToD3D12(desc.minFilter);
            const D3D12_FILTER_TYPE magFilter = ToD3D12(desc.magFilter);
            const D3D12_FILTER_TYPE mipFilter = ToD3D12(desc.mipFilter);
            const D3D12_FILTER_REDUCTION_TYPE reductionType = ToD3D12(desc.reductionType);

            D3D12_SAMPLER_DESC d3d12Desc{};
            if (desc.maxAnisotropy > 1.0f)
            {
                d3d12Desc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
            }
            else
            {
                d3d12Desc.Filter = D3D12_ENCODE_BASIC_FILTER(minFilter, magFilter, mipFilter, reductionType);
            }

            d3d12Desc.AddressU = ToD3D12(desc.addressModeU);
            d3d12Desc.AddressV = ToD3D12(desc.addressModeV);
            d3d12Desc.AddressW = ToD3D12(desc.addressModeW);
            d3d12Desc.MipLODBias = 0.0f;
            d3d12Desc.MaxAnisotropy = std::min<uint16_t>(std::max((UINT)desc.maxAnisotropy, 1U), 16u);
            if (reductionType != D3D12_FILTER_REDUCTION_TYPE_COMPARISON)
            {
                d3d12Desc.ComparisonFunc = D3D12_COMPARISON_FUNC_NONE;
            }
            else
            {
                d3d12Desc.ComparisonFunc = ToD3D12(desc.compareFunction);
            }
            switch (desc.borderColor)
            {
                case RHISamplerBorderColor::FloatOpaqueBlack:
                case RHISamplerBorderColor::UintOpaqueBlack:
                    d3d12Desc.BorderColor[0] = 0.0f;
                    d3d12Desc.BorderColor[1] = 0.0f;
                    d3d12Desc.BorderColor[2] = 0.0f;
                    d3d12Desc.BorderColor[3] = 1.0f;
                    break;

                case RHISamplerBorderColor::FloatOpaqueWhite:
                case RHISamplerBorderColor::UintOpaqueWhite:
                    d3d12Desc.BorderColor[0] = 1.0f;
                    d3d12Desc.BorderColor[1] = 1.0f;
                    d3d12Desc.BorderColor[2] = 1.0f;
                    d3d12Desc.BorderColor[3] = 1.0f;
                    break;

                default:
                    d3d12Desc.BorderColor[0] = 0.0f;
                    d3d12Desc.BorderColor[1] = 0.0f;
                    d3d12Desc.BorderColor[2] = 0.0f;
                    d3d12Desc.BorderColor[3] = 0.0f;
                    break;
            }
            d3d12Desc.MinLOD = desc.lodMinClamp;
            d3d12Desc.MaxLOD = desc.lodMaxClamp;
            return d3d12Desc;
        }

        D3D12_STATIC_SAMPLER_DESC ToD3D12StaticSamplerDesc(const RHISamplerDesc& desc,
            uint32_t shaderRegister, uint32_t registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
        {
            D3D12_SAMPLER_DESC samplerDesc = ToD3D12SamplerDesc(desc);

            D3D12_STATIC_SAMPLER_DESC staticDesc = { };
            staticDesc.Filter = samplerDesc.Filter;
            staticDesc.AddressU = samplerDesc.AddressU;
            staticDesc.AddressV = samplerDesc.AddressV;
            staticDesc.AddressW = samplerDesc.AddressW;
            staticDesc.MipLODBias = samplerDesc.MipLODBias;
            staticDesc.MaxAnisotropy = samplerDesc.MaxAnisotropy;
            staticDesc.ComparisonFunc = samplerDesc.ComparisonFunc;
            staticDesc.MinLOD = samplerDesc.MinLOD;
            staticDesc.MaxLOD = samplerDesc.MaxLOD;
            staticDesc.ShaderRegister = shaderRegister;
            staticDesc.RegisterSpace = registerSpace;
            staticDesc.ShaderVisibility = visibility;

            switch (desc.borderColor)
            {
                case RHISamplerBorderColor::FloatOpaqueBlack:
                    staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
                    break;

                case RHISamplerBorderColor::UintOpaqueBlack:
                    staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT;
                    break;

                case RHISamplerBorderColor::FloatOpaqueWhite:
                    staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
                    break;

                case RHISamplerBorderColor::UintOpaqueWhite:
                    staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT;
                    break;

                default:
                    staticDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                    break;
            }

            return staticDesc;
        }

        constexpr D3D12_QUERY_HEAP_TYPE ToD3D12QueryHeapType(RHIQueryType type)
        {
            switch (type)
            {
                case RHIQueryType::Occlusion:
                case RHIQueryType::BinaryOcclusion:
                    return D3D12_QUERY_HEAP_TYPE_OCCLUSION;

                case RHIQueryType::Timestamp:
                    return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
                    //return D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;

                case RHIQueryType::PipelineStatistics:
                    return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;

                default:
                    return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            }
        }

        constexpr D3D12_QUERY_TYPE ToD3D12QueryType(RHIQueryType type)
        {
            switch (type)
            {
                case RHIQueryType::Occlusion:           return D3D12_QUERY_TYPE_OCCLUSION;
                case RHIQueryType::BinaryOcclusion:     return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
                case RHIQueryType::Timestamp:           return D3D12_QUERY_TYPE_TIMESTAMP;
                case RHIQueryType::PipelineStatistics:  return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
                default:                                return D3D12_QUERY_TYPE_TIMESTAMP;
            }
        }

        constexpr uint32_t GetQueryResultSize(RHIQueryType type)
        {
            switch (type)
            {
                case RHIQueryType::Occlusion:
                case RHIQueryType::BinaryOcclusion:
                case RHIQueryType::Timestamp:
                    return sizeof(uint64_t);

                case RHIQueryType::PipelineStatistics:
                    return sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS);

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr D3D12_SHADER_COMPONENT_MAPPING ToD3D12TextureSwizzle(RHITextureSwizzle value)
        {
            switch (value)
            {
                default:
                case RHITextureSwizzle::Red:    return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
                case RHITextureSwizzle::Green:  return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
                case RHITextureSwizzle::Blue:   return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;
                case RHITextureSwizzle::Alpha:  return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;
                case RHITextureSwizzle::Zero:   return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
                case RHITextureSwizzle::One:    return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
            }
        }

        constexpr UINT ToD3D12Swizzle(TextureSwizzleChannels value)
        {
            return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(ToD3D12TextureSwizzle(value.red), ToD3D12TextureSwizzle(value.green), ToD3D12TextureSwizzle(value.blue), ToD3D12TextureSwizzle(value.alpha));
        }

        constexpr D3D_PRIMITIVE_TOPOLOGY ToD3DPrimitiveTopology(RHIPrimitiveTopology type)
        {
            switch (type)
            {
                case RHIPrimitiveTopology::PointList:
                    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case RHIPrimitiveTopology::LineList:
                    return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case RHIPrimitiveTopology::LineStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case RHIPrimitiveTopology::TriangleList:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case RHIPrimitiveTopology::TriangleStrip:
                    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

                default:
                    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
            }
        }

        constexpr uint32_t ToD3D12(RHITextureSampleCount count)
        {
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count1) == 1, "TextureSampleCount missmatch");
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count2) == 2, "TextureSampleCount missmatch");
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count4) == 4, "TextureSampleCount missmatch");
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count8) == 8, "TextureSampleCount missmatch");
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count16) == 16, "TextureSampleCount missmatch");
            static_assert(static_cast<uint32_t>(RHITextureSampleCount::Count32) == 32, "TextureSampleCount missmatch");
            //static_assert(static_cast<uint32_t>(TextureSampleCount::Count64) == 64, "TextureSampleCount missmatch");

            return static_cast<uint32_t>(count);
        }

        constexpr DXGI_FORMAT ToDxgiSwapChainFormat(PixelFormat format)
        {
            // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
            switch (format)
            {
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

                default:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
            }
        }

        constexpr uint32_t PresentModeToBufferCount(RHIPresentMode mode)
        {
            switch (mode)
            {
                case RHIPresentMode::Immediate:
                case RHIPresentMode::Fifo:
                    return 2;
                case RHIPresentMode::Mailbox:
                    return 3;
                default:
                    return 2;
            }
        }

        constexpr uint32_t PresentModeToSyncInterval(RHIPresentMode mode)
        {
            switch (mode)
            {
                case RHIPresentMode::Immediate:
                case RHIPresentMode::Mailbox:
                    return 0u;

                case RHIPresentMode::Fifo:
                default:
                    return 1u;
            }
        }

        constexpr D3D12_SHADING_RATE ToD3D12(RHIShadingRate value)
        {
            switch (value)
            {
                case RHIShadingRate::Rate1x1:   return D3D12_SHADING_RATE_1X1;
                case RHIShadingRate::Rate1x2:   return D3D12_SHADING_RATE_1X2;
                case RHIShadingRate::Rate2x1:   return D3D12_SHADING_RATE_2X1;
                case RHIShadingRate::Rate2x2:   return D3D12_SHADING_RATE_2X2;
                case RHIShadingRate::Rate2x4:   return D3D12_SHADING_RATE_2X4;
                case RHIShadingRate::Rate4x2:   return D3D12_SHADING_RATE_4X2;
                case RHIShadingRate::Rate4x4:   return D3D12_SHADING_RATE_4X4;
                default:                        return D3D12_SHADING_RATE_1X1;
            }
        }

        [[maybe_unused]] constexpr uint32_t GetPlaneSliceCount(DXGI_FORMAT format)
        {
            switch (format)
            {
                case DXGI_FORMAT_D24_UNORM_S8_UINT:
                case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                    return 2;
                default:
                    return 1;
            }
        }

        [[nodiscard]] constexpr RHIShaderModel FromD3D12(D3D_SHADER_MODEL value)
        {
            switch (value)
            {
                case D3D_SHADER_MODEL_6_1:  return RHIShaderModel::Model_6_1;
                case D3D_SHADER_MODEL_6_2:  return RHIShaderModel::Model_6_2;
                case D3D_SHADER_MODEL_6_3:  return RHIShaderModel::Model_6_3;
                case D3D_SHADER_MODEL_6_4:  return RHIShaderModel::Model_6_4;
                case D3D_SHADER_MODEL_6_5:  return RHIShaderModel::Model_6_5;
                case D3D_SHADER_MODEL_6_6:  return RHIShaderModel::Model_6_6;
                case D3D_SHADER_MODEL_6_7:  return RHIShaderModel::Model_6_7;
                case D3D_SHADER_MODEL_6_8:  return RHIShaderModel::Model_6_8;
                case D3D_SHADER_MODEL_6_9:  return RHIShaderModel::Model_6_9;

                default:
                case D3D_SHADER_MODEL_6_0:
                    return RHIShaderModel::Model_6_0;
            }
        }
        [[nodiscard]] constexpr RHIRayTracingTier FromD3D12(D3D12_RAYTRACING_TIER value)
        {
            switch (value)
            {
                case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
                    return RHIRayTracingTier::NotSupported;

                case D3D12_RAYTRACING_TIER_1_0:
                    return RHIRayTracingTier::Tier1;

                case D3D12_RAYTRACING_TIER_1_1:
                    return RHIRayTracingTier::Tier2;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        [[nodiscard]] constexpr RHIMeshShaderTier FromD3D12(D3D12_MESH_SHADER_TIER value)
        {
            switch (value)
            {
                case D3D12_MESH_SHADER_TIER_NOT_SUPPORTED:
                    return RHIMeshShaderTier::NotSupported;

                case D3D12_MESH_SHADER_TIER_1:
                    return RHIMeshShaderTier::Tier1;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        constexpr uint32_t GetPlaneSlice(DXGI_FORMAT format, RHITextureAspect aspect)
        {
            switch (aspect)
            {
                case RHITextureAspect::All:
                case RHITextureAspect::DepthOnly:
                    return 0;
                case RHITextureAspect::StencilOnly:
                    switch (format)
                    {
                        case DXGI_FORMAT_D24_UNORM_S8_UINT:
                        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                            return 1;
                        default:
                            return 0;
                    }
                default:
                    ALIMER_UNREACHABLE();
                    return 0;
            }
        }

        struct D3D12BufferStateMapping final
        {
            RHIBufferStates state = RHIBufferStates::Undefined;
            D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;
            D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;

            D3D12BufferStateMapping() = default;

            D3D12BufferStateMapping(RHIBufferStates state_, D3D12_BARRIER_SYNC sync_, D3D12_BARRIER_ACCESS access_)
                : state(state_)
                , sync(sync_)
                , access(access_)
            {}
        };

        struct D3D12TextureLayoutMapping final
        {
            D3D12_BARRIER_LAYOUT layout;
            D3D12_BARRIER_SYNC sync;
            D3D12_BARRIER_ACCESS access;

            D3D12TextureLayoutMapping(D3D12_BARRIER_LAYOUT layout_, D3D12_BARRIER_SYNC sync_, D3D12_BARRIER_ACCESS access_)
                : layout(layout_)
                , sync(sync_)
                , access(access_)
            {}
        };

        static const D3D12BufferStateMapping g_BufferStateMap[] =
        {
            { RHIBufferStates::CopyDest, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_DEST },
            { RHIBufferStates::CopySource, D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_ACCESS_COPY_SOURCE },
            { RHIBufferStates::ShaderResource, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_SHADER_RESOURCE },
            { RHIBufferStates::UnorderedAccess, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_UNORDERED_ACCESS },
            { RHIBufferStates::VertexBuffer, D3D12_BARRIER_SYNC_VERTEX_SHADING, D3D12_BARRIER_ACCESS_VERTEX_BUFFER },
            { RHIBufferStates::IndexBuffer, D3D12_BARRIER_SYNC_INDEX_INPUT, D3D12_BARRIER_ACCESS_INDEX_BUFFER },
            { RHIBufferStates::ConstantBuffer, D3D12_BARRIER_SYNC_ALL_SHADING, D3D12_BARRIER_ACCESS_CONSTANT_BUFFER },
            //{ RHIBufferStates::Predication, D3D12_BARRIER_SYNC_PREDICATION, D3D12_BARRIER_ACCESS_PREDICATION },
            #if TODO
            { RHIBufferStates::IndirectArgument, VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT },
            { RHIBufferStates::StreamOut, VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT, VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT },
            { RHIBufferStates::AccelerationStructureRead, VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR },
            { RHIBufferStates::AccelerationStructureWrite, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR },
            { RHIBufferStates::AccelerationStructureBuildInput, VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR},
            { RHIBufferStates::OpacityMicromapWrite, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_MICROMAP_WRITE_BIT_EXT },
            { RHIBufferStates::OpacityMicromapBuildInput, VK_PIPELINE_STAGE_2_MICROMAP_BUILD_BIT_EXT, VK_ACCESS_2_SHADER_READ_BIT },
             #endif // TODO
        };

        D3D12BufferStateMapping ConvertBufferState(RHIBufferStates state)
        {
            D3D12BufferStateMapping result = {};

            constexpr uint32_t numStateBits = sizeof(g_BufferStateMap) / sizeof(g_BufferStateMap[0]);

            uint32_t stateTmp = uint32_t(state);
            uint32_t bitIndex = 0;

            while (stateTmp != 0 && bitIndex < numStateBits)
            {
                uint32_t bit = (1 << bitIndex);

                if (stateTmp & bit)
                {
                    const D3D12BufferStateMapping& mapping = g_BufferStateMap[bitIndex];

                    ALIMER_ASSERT(uint32_t(mapping.state) == bit);

                    result.state |= mapping.state;
                    result.sync |= mapping.sync;
                    result.access |= mapping.access;

                    stateTmp &= ~bit;
                }

                bitIndex++;
            }

            ALIMER_ASSERT(result.state == state);

            return result;
        }

        D3D12TextureLayoutMapping ConvertTextureLayout(RHITextureLayout layout)
        {
            switch (layout)
            {
                case RHITextureLayout::Undefined:
                    return {
                        D3D12_BARRIER_LAYOUT_COMMON,
                        D3D12_BARRIER_SYNC_NONE,
                        D3D12_BARRIER_ACCESS_COMMON
                    };

                case RHITextureLayout::CopySource:
                    return {
                        D3D12_BARRIER_LAYOUT_COPY_SOURCE,
                        D3D12_BARRIER_SYNC_COPY,
                        D3D12_BARRIER_ACCESS_COPY_SOURCE
                    };

                case RHITextureLayout::CopyDest:
                    return {
                        D3D12_BARRIER_LAYOUT_COPY_DEST,
                        D3D12_BARRIER_SYNC_COPY,
                        D3D12_BARRIER_ACCESS_COPY_DEST
                    };

                case RHITextureLayout::ResolveSource:
                    return {
                        D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE,
                        D3D12_BARRIER_SYNC_RESOLVE,
                        D3D12_BARRIER_ACCESS_RESOLVE_SOURCE
                    };

                case RHITextureLayout::ResolveDest:
                    return {
                        D3D12_BARRIER_LAYOUT_RESOLVE_DEST,
                        D3D12_BARRIER_SYNC_RESOLVE,
                        D3D12_BARRIER_ACCESS_RESOLVE_DEST
                    };

                case RHITextureLayout::ShaderResource:
                    return {
                        D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,
                        D3D12_BARRIER_SYNC_ALL_SHADING,
                        D3D12_BARRIER_ACCESS_SHADER_RESOURCE
                    };

                case RHITextureLayout::UnorderedAccess:
                    return {
                        D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
                        D3D12_BARRIER_SYNC_ALL_SHADING,
                        D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
                    };

                case RHITextureLayout::RenderTarget:
                    return {
                        D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                        D3D12_BARRIER_SYNC_RENDER_TARGET,
                        D3D12_BARRIER_ACCESS_RENDER_TARGET };

                case RHITextureLayout::DepthWrite:
                    return {
                        D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,
                        D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                        D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE
                    };

                case RHITextureLayout::DepthRead:
                    return {
                        D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ,
                        D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                        D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ
                    };

                case RHITextureLayout::Present:
                    return {
                        D3D12_BARRIER_LAYOUT_PRESENT,
                        D3D12_BARRIER_SYNC_ALL,
                        D3D12_BARRIER_ACCESS_COMMON
                    };

                case RHITextureLayout::ShadingRateSurface:
                    return {
                        D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE,
                        D3D12_BARRIER_SYNC_PIXEL_SHADING,
                        D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE
                    };


                default:
                    ALIMER_UNREACHABLE();
            }
        }

        D3D12_RESOURCE_STATES ConvertBufferStateLegacy(RHIBufferStates stateBits, RHIQueueType queueType)
        {
            D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

            if ((stateBits & RHIBufferStates::CopyDest) != 0)
                result |= D3D12_RESOURCE_STATE_COPY_DEST;

            if ((stateBits & RHIBufferStates::CopySource) != 0)
                result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

            if ((stateBits & RHIBufferStates::ShaderResource) != 0)
            {
                result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                if (queueType == RHIQueueType::Graphics)
                    result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            }

            if ((stateBits & RHIBufferStates::UnorderedAccess) != 0)
                result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            if ((stateBits & RHIBufferStates::VertexBuffer) != 0)
                result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            if ((stateBits & RHIBufferStates::IndexBuffer) != 0) result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
            if ((stateBits & RHIBufferStates::ConstantBuffer) != 0) result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            //if ((stateBits & RHIBufferStates::Predication) != 0) result |= D3D12_RESOURCE_STATE_PREDICATION;
#if TODO
            if ((stateBits & RHIBufferStates::IndirectArgument) != 0) result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            if ((stateBits & RHIBufferStates::StreamOut) != 0) result |= D3D12_RESOURCE_STATE_STREAM_OUT;
            if ((stateBits & RHIBufferStates::AccelerationStructureRead) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & RHIBufferStates::AccelerationStructureWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & RHIBufferStates::AccelerationStructureBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
            if ((stateBits & RHIBufferStates::ShadingRateSurface) != 0) result |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
            if ((stateBits & RHIBufferStates::OpacityMicromapBuildInput) != 0) result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            if ((stateBits & RHIBufferStates::OpacityMicromapWrite) != 0) result |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif // TODO


            return result;
        }

        D3D12_RESOURCE_STATES ConvertTextureLayoutLegacy(RHITextureLayout layout)
        {
            switch (layout)
            {
                case RHITextureLayout::Undefined:
                    return D3D12_RESOURCE_STATE_COMMON;

                case RHITextureLayout::CopySource:
                    return D3D12_RESOURCE_STATE_COPY_SOURCE;

                case RHITextureLayout::CopyDest:
                    return D3D12_RESOURCE_STATE_COPY_DEST;

                case RHITextureLayout::ShaderResource:
                    return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

                case RHITextureLayout::UnorderedAccess:
                    return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

                case RHITextureLayout::RenderTarget:
                    return D3D12_RESOURCE_STATE_RENDER_TARGET;

                case RHITextureLayout::DepthWrite:
                    return D3D12_RESOURCE_STATE_DEPTH_WRITE;

                case RHITextureLayout::DepthRead:
                    return D3D12_RESOURCE_STATE_DEPTH_READ;

                case RHITextureLayout::Present:
                    return D3D12_RESOURCE_STATE_PRESENT;

                case RHITextureLayout::ShadingRateSurface:
                    return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

                default:
                    ALIMER_UNREACHABLE();
            }
        }

        inline void __stdcall DebugMessageCallback(
            D3D12_MESSAGE_CATEGORY Category,
            D3D12_MESSAGE_SEVERITY Severity,
            D3D12_MESSAGE_ID ID,
            LPCSTR pDescription,
            void* pContext)
        {
            std::string message = pDescription;
            if (Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION || Severity == D3D12_MESSAGE_SEVERITY_ERROR)
            {
                LOGE("{}", message);
                ALIMER_UNREACHABLE();
            }
            else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
            {
                LOGW("{}", message);
            }
            else
            {
                LOGI(message);
            }
        }
    }

    using DescriptorIndex = uint32_t;
    using RootParameterIndex = uint32_t;

    static constexpr DescriptorIndex kInvalidDescriptorIndex = ~0u;

    class D3D12Device;
    class D3D12Adapter;
    class D3D12Factory;

    struct D3D12Resource
    {
        D3D12Device* device = nullptr;
        ID3D12Resource* handle = nullptr;
        D3D12MA::Allocation* allocation = nullptr;
        bool immutableState = false;
    };

    struct D3D12Buffer final : public RHIBuffer, public D3D12Resource
    {
    public:
        uint64_t allocatedSize{};
        D3D12_GPU_VIRTUAL_ADDRESS deviceAddress{};
        void* pMappedData = nullptr;
        HANDLE sharedHandle = nullptr;
        mutable RHIBufferStates currentState = RHIBufferStates::Undefined;

        explicit D3D12Buffer(D3D12Device* device_, const RHIBufferDesc& desc_)
            : RHIBuffer(desc_)
        {
            device = device_;
        }

        ~D3D12Buffer() override;

        uint64_t GetAllocatedSize() const { return allocatedSize; }
        void SetLabel(const char* label) override
        {
            auto wName = ToUtf16(label);
            handle->SetName(wName.c_str());
            allocation->SetName(wName.c_str());
        }

        void* GetMappedData() const override { return pMappedData; }
        GPUAddress GetGPUAddress() const override { return deviceAddress; }
        RHINativeHandle GetNativeHandle(RHINativeHandleType type) override;
    };

    struct D3D12Texture final : public RHITexture, public D3D12Resource
    {
        mutable UnorderedMap<size_t, DescriptorIndex> RTVs;
        mutable UnorderedMap<size_t, DescriptorIndex> DSVs;

        Vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footPrints;
        Vector<uint64_t> rowSizesInBytes;
        Vector<uint32_t> numRows;
        DXGI_FORMAT dxgiFormat{ DXGI_FORMAT_UNKNOWN };
        uint64_t allocatedSize{};
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddress{};
        void* pMappedData = nullptr;
        HANDLE sharedHandle = nullptr;
        uint32_t numSubResources = 0;
        mutable Vector<RHITextureLayout> subResourcesStates;

        explicit D3D12Texture(D3D12Device* device_, const RHITextureDesc& desc)
            : RHITexture(desc)
        {
            device = device_;
        }

        ~D3D12Texture() override;

        RHITextureViewRef CreateView(const RHITextureViewDesc& desc) const override;
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(const RHITextureViewDesc& desc, DXGI_FORMAT rtvFormat) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(const RHITextureViewDesc& desc, DXGI_FORMAT dsvFormat, bool readOnly) const;
        uint64_t GetAllocatedSize() const { return allocatedSize; }
        void SetLabel(const char* label) override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
    };

    struct D3D12TextureView final : public RHITextureView
    {
        DXGI_FORMAT RTVFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT DSVFormat = DXGI_FORMAT_UNKNOWN;

        D3D12_CPU_DESCRIPTOR_HANDLE RTV = {};
        D3D12_CPU_DESCRIPTOR_HANDLE DSV = {};
        D3D12_CPU_DESCRIPTOR_HANDLE DSVReadOnly = {};

        explicit D3D12TextureView(const D3D12Texture* texture_, const RHITextureViewDesc& desc)
            : RHITextureView(texture_, desc)
        {

        }

        D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDescriptor() const;
        D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDescriptor() const;
    };

    struct D3D12Sampler final : public RHISampler
    {
        D3D12Device* device = nullptr;
        RHISamplerDesc desc;
        D3D12_SAMPLER_DESC handle{};

        ~D3D12Sampler() override;
        const RHISamplerDesc& GetDesc() const override { return desc; }
    };

    struct D3D12ShaderModule final : public RHIShaderModule
    {
        D3D12Device* device = nullptr;
        uint8_t* pByteCode = nullptr;
        D3D12_SHADER_BYTECODE bytecode = {};

        ~D3D12ShaderModule() override;
    };

    struct D3D12ComputePipeline final : public RHIComputePipeline
    {
        D3D12Device* device = nullptr;
        ID3D12PipelineState* handle = nullptr;

        ~D3D12ComputePipeline() override;
        void SetLabel(const char* label) override;
    };

    struct D3D12RenderPipeline final : public RHIRenderPipeline
    {
        D3D12Device* device = nullptr;
        ID3D12PipelineState* handle = nullptr;

        // Render Pipeline Only
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        uint32_t numVertexBindings = 0;
        uint32_t strides[kMaxVertexBuffers] = {};

        ~D3D12RenderPipeline() override;
        void SetLabel(const char* label) override;
    };

    struct D3D12QueryHeap final : public RHIQueryHeap
    {
        D3D12Device* device = nullptr;
        RHIQueryHeapDesc desc;
        ID3D12QueryHeap* handle = nullptr;
        D3D12_QUERY_TYPE queryType = D3D12_QUERY_TYPE_OCCLUSION;
        uint32_t queryResultSize = 0;

        ~D3D12QueryHeap() override;
        void SetLabel(const char* label) override;

        uint32_t GetCount() const override { return desc.count; }
        RHIQueryType GetType() const override { return desc.type; }
    };

    struct D3D12Surface final : public RHISurface
    {
        D3D12Device* device = nullptr;

        IDXGISwapChain3* handle = nullptr;
        uint32_t syncInterval = 1u;
        uint32_t presentFlags = 0;
        uint32_t backBufferCount = 0;
        uint32_t backBufferIndex = 0;
        PixelFormat colorFormat = PixelFormat::Undefined;
        std::vector<SharedPtr<D3D12Texture>> backbufferTextures;

        ~D3D12Surface() override;


        RHIStatus GetCapabilities(RHIAdapter* adapter, RHISurfaceCapabilities* capabilities) override;
        void Configure(RHIDevice* device, const RHISurfaceConfig& config) override;
        void Unconfigure() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;
        void SetLabel(const char* label) override;
    };

    class D3D12CommandBuffer;

    class D3D12ComputePassEncoder final : public RHIComputePassEncoder
    {
        friend class D3D12CommandBuffer;

    public:
        D3D12ComputePassEncoder(D3D12Device* device, D3D12CommandBuffer* commandBuffer);
        ~D3D12ComputePassEncoder() override;

        void Begin(const RHIComputePassDesc& desc);

        void PushDebugGroup(std::string_view groupLabel) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view markerLabel) override;

        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, const RHIBuffer* destinationBuffer) override;
        void CopyBufferToBuffer(const RHIBuffer* sourceBuffer, uint64_t sourceOffset, const RHIBuffer* destinationBuffer, uint64_t destinationOffset, uint64_t size) override;

        void SetPipeline(RHIComputePipeline* pipeline) override;
        void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) override;
        void DispatchCore(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        void DispatchIndirectCore(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;

        void End() override;
        RHICommandBuffer* GetCommandBuffer() const override;

    private:
        void ClearState();
        void PrepareDispatch();

        D3D12Device* _device;
        D3D12CommandBuffer* _commandBuffer;
        ID3D12GraphicsCommandList* _d3dCommandList = nullptr;
        bool _hasLabel{ false };
        SharedPtr<D3D12ComputePipeline> _currentPipeline;
    };

    class D3D12RenderPassEncoder final : public RHIRenderPassEncoder
    {
        friend class D3D12CommandBuffer;

    public:
        D3D12RenderPassEncoder(D3D12Device* device, D3D12CommandBuffer* commandBuffer);
        ~D3D12RenderPassEncoder() override;

        void Begin(const RHIRenderPassDesc& desc);

        void PushDebugGroup(std::string_view groupLabel) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view markerLabel) override;

        void SetViewport(const RHIViewport& viewport) override;
        void SetViewports(const RHIViewport* viewports, uint32_t count) override;
        void SetScissorRect(const RHIScissorRect& rect) override;
        void SetScissorRects(const RHIScissorRect* scissorRects, uint32_t count) override;
        void SetStencilReference(uint32_t referenceValue) override;
        void SetBlendColor(const Color& color) override;
        void SetShadingRate(RHIShadingRate rate) override;
        void SetDepthBounds(float minBounds, float maxBounds) override;

        void SetPipeline(RHIRenderPipeline* pipeline) override;
        void SetPushConstantsCore(const void* data, uint32_t size, uint32_t offset) override;

        void SetVertexBuffer(uint32_t slot, const RHIBuffer* buffer, uint64_t offset) override;
        void SetVertexBuffers(uint32_t slot, uint32_t count, const RHIBuffer** buffers, const uint64_t* offsets) override;
        void SetIndexBuffer(const RHIBuffer* buffer, uint64_t offset, RHIIndexFormat format) override;

        void PrepareDraw();
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) override;
        void DrawIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawIndexedIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawMesh(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;
        void DrawMeshIndirect(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset) override;
        void DrawMeshIndirectCount(const RHIBuffer* indirectBuffer, uint64_t indirectBufferOffset, const RHIBuffer* countBuffer, uint64_t countBufferOffset, uint32_t maxCount) override;

        void End() override;

        RHICommandBuffer* GetCommandBuffer() const override;

    private:
        void ClearState();

        D3D12Device* _device;
        D3D12CommandBuffer* _commandBuffer;
        ID3D12GraphicsCommandList* _d3dCommandList = nullptr;
        bool _hasLabel{ false };
        RHIShadingRate _currentShadingRate{ RHIShadingRate::Invalid };
        SharedPtr<D3D12RenderPipeline> _currentPipeline;
    };

    class D3D12CommandBuffer final : public RHICommandBuffer
    {
        friend class D3D12Device;

    public:
        D3D12CommandBuffer(D3D12Device* device, RHIQueueType queueType, uint32_t id);
        ~D3D12CommandBuffer() override;

        void Clear();
        void Begin(uint32_t frameIndex, std::string_view label);
        ID3D12CommandList* End();

        void BufferBarrier(const D3D12Buffer* buffer, RHIBufferStates newState, bool commit = false);
        void TextureBarrier(const D3D12Texture* resource, RHITextureLayout newLayout, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool commit = false);
        void InsertUAVBarrier(const D3D12Resource* resource, bool commit = false);
        void CommitBarriers();

        void PushDebugGroup(std::string_view name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(std::string_view name) override;

        void BeginQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void EndQuery(const RHIQueryHeap* heap, uint32_t index) override;
        void ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset) override;
        void ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count) override;

        /* GraphicsContext */
        RHITexture* AcquireSurfaceTexture(RHISurface* surface) override;

        RHIComputePassEncoder* BeginComputePassCore(const RHIComputePassDesc& desc) override;
        RHIRenderPassEncoder* BeginRenderPassCore(const RHIRenderPassDesc& desc) override;

        RHIDevice* GetDevice() const override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;

    private:
        static constexpr uint32_t kMaxBarrierCount = 16;

        D3D12Device* device;
        RHIQueueType queueType;
        uint32_t id;

        ID3D12CommandAllocator* commandAllocators[kNumFramesInFlight] = {};
        ID3D12GraphicsCommandList6* commandList6 = nullptr;
        ID3D12GraphicsCommandList7* commandList7 = nullptr;

        std::vector<D3D12CommandBuffer*> waits;
        std::atomic_bool hasPendingWaits{ false };
        bool hasLabel = false;

        uint32_t numBarriersToCommit = 0;
        std::vector<D3D12_GLOBAL_BARRIER> globalBarriers;
        std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;
        std::vector<D3D12_BUFFER_BARRIER> bufferBarriers;
        // Legacy barriers
        D3D12_RESOURCE_BARRIER barriers[kMaxBarrierCount] = {};

        D3D12_VERTEX_BUFFER_VIEW vboViews[kMaxVertexBuffers] = {};
        std::vector<SharedPtr<D3D12Surface>> presentSwapChains;
    };

    struct D3D12Queue final : public RHIQueue
    {
        D3D12Device* device = nullptr;
        RHIQueueType queueType = RHIQueueType::Count;
        ID3D12CommandQueue* handle = nullptr;
        ID3D12Fence* fence = nullptr;
        ID3D12Fence* frameFences[kNumFramesInFlight] = {};
        std::vector<ID3D12CommandList*> submitCommandLists;

        RHIQueueType GetType() const override { return queueType; }
    };

    struct D3D12UploadContext final
    {
        ID3D12CommandAllocator* commandAllocator = nullptr;
        ID3D12GraphicsCommandList* commandList = nullptr;
        ID3D12Fence* fence = nullptr;
        uint64_t fenceValueSignaled = 0;
        SharedPtr<D3D12Buffer> uploadBuffer;
        void* uploadBufferData = nullptr;

        inline bool IsValid() const { return commandList != nullptr; }
        inline bool IsCompleted() const { return fence->GetCompletedValue() >= fenceValueSignaled; }
    };

    class D3D12DescriptorAllocator final
    {
    public:
        void Init(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType_, uint32_t numDescriptors_)
        {
            device = device_;
            heapType = heapType_;
            shaderVisible = heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            stride = device_->GetDescriptorHandleIncrementSize(heapType);

            ThrowIfFailed(AllocateResources(numDescriptors_));
        }

        void Shutdown()
        {
            heap = nullptr;
            shaderVisibleHeap = nullptr;
        }

        DescriptorIndex AllocateDescriptors(uint32_t count = 1u)
        {
            std::lock_guard lockGuard(mutex);

            DescriptorIndex foundIndex = 0;
            uint32_t freeCount = 0;
            bool found = false;

            // Find a contiguous range of 'count' indices for which m_AllocatedDescriptors[index] is false
            for (DescriptorIndex index = searchStart; index < numDescriptors; index++)
            {
                if (allocatedDescriptors[index])
                    freeCount = 0;
                else
                    freeCount += 1;

                if (freeCount >= count)
                {
                    foundIndex = index - count + 1;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                foundIndex = numDescriptors;

                if (FAILED(Grow(numDescriptors + count)))
                {
                    LOGE("Failed to grow a descriptor heap!");
                    return kInvalidDescriptorIndex;
                }
            }

            for (DescriptorIndex index = foundIndex; index < foundIndex + count; index++)
            {
                allocatedDescriptors[index] = true;
            }

            numAllocatedDescriptors += count;

            searchStart = foundIndex + count;
            return foundIndex;
        }

        void ReleaseDescriptors(DescriptorIndex baseIndex, uint32_t count)
        {
            if (count == 0)
                return;

            std::lock_guard lockGuard(mutex);

            for (DescriptorIndex index = baseIndex; index < baseIndex + count; index++)
            {
#ifdef _DEBUG
                if (!allocatedDescriptors[index])
                {
                    LOGE("Attempted to release an un-allocated descriptor");
                }
#endif

                allocatedDescriptors[index] = false;
            }

            numAllocatedDescriptors -= count;

            if (searchStart > baseIndex)
                searchStart = baseIndex;
        }

        void ReleaseDescriptor(DescriptorIndex index)
        {
            ReleaseDescriptors(index, 1);
        }

        void CopyToShaderVisibleHeap(DescriptorIndex index, uint32_t count = 1)
        {
            device->CopyDescriptorsSimple(count, GetCpuHandleShaderVisible(index), GetCpuHandle(index), heapType);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(DescriptorIndex index)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = startCpuHandle;
            handle.ptr += index * stride;
            return handle;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleShaderVisible(DescriptorIndex index)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = startCpuHandleShaderVisible;
            handle.ptr += index * stride;
            return handle;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(DescriptorIndex index)
        {
            D3D12_GPU_DESCRIPTOR_HANDLE handle = startGpuHandleShaderVisible;
            handle.ptr += index * stride;
            return handle;
        }

        [[nodiscard]] ID3D12DescriptorHeap* GetHeap() const { return heap.Get(); }
        [[nodiscard]] ID3D12DescriptorHeap* GetShaderVisibleHeap() const { return shaderVisibleHeap.Get(); }
        [[nodiscard]] uint32_t GetStride() const { return stride; }

    private:
        ID3D12Device* device = nullptr;
        ComPtr<ID3D12DescriptorHeap> heap;
        ComPtr<ID3D12DescriptorHeap> shaderVisibleHeap;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        uint32_t numDescriptors = 0;
        bool shaderVisible = true;
        uint32_t stride = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE startCpuHandle = { 0 };
        D3D12_CPU_DESCRIPTOR_HANDLE startCpuHandleShaderVisible = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE startGpuHandleShaderVisible = { 0 };
        std::vector<bool> allocatedDescriptors;
        DescriptorIndex searchStart = 0;
        uint32_t numAllocatedDescriptors = 0;
        std::mutex mutex;

        HRESULT AllocateResources(uint32_t numDescriptors_)
        {
            heap = nullptr;
            shaderVisibleHeap = nullptr;
            numDescriptors = numDescriptors_;

            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = heapType;
            heapDesc.NumDescriptors = numDescriptors;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            HRESULT hr = device->CreateDescriptorHeap(&heapDesc, PPV_ARGS(heap));

            if (FAILED(hr))
                return hr;

            if (shaderVisible)
            {
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                hr = device->CreateDescriptorHeap(&heapDesc, PPV_ARGS(shaderVisibleHeap));

                if (FAILED(hr))
                    return hr;

                startCpuHandleShaderVisible = shaderVisibleHeap->GetCPUDescriptorHandleForHeapStart();
                startGpuHandleShaderVisible = shaderVisibleHeap->GetGPUDescriptorHandleForHeapStart();
            }

            startCpuHandle = heap->GetCPUDescriptorHandleForHeapStart();
            allocatedDescriptors.resize(numDescriptors);

            return S_OK;
        }

        HRESULT Grow(uint32_t minRequiredSize)
        {
            uint32_t oldSize = numDescriptors;
            uint32_t newSize = NextPowerOfTwo(minRequiredSize);

            ComPtr<ID3D12DescriptorHeap> oldHeap = heap;

            HRESULT hr = AllocateResources(newSize);

            if (FAILED(hr))
                return hr;

            device->CopyDescriptorsSimple(oldSize, startCpuHandle, oldHeap->GetCPUDescriptorHandleForHeapStart(), heapType);

            if (shaderVisibleHeap != nullptr)
            {
                device->CopyDescriptorsSimple(oldSize, startCpuHandleShaderVisible, oldHeap->GetCPUDescriptorHandleForHeapStart(), heapType);
            }

            return S_OK;
        }
    };

    static HMODULE lib_dxgi = nullptr;
    static HMODULE lib_d3d12 = nullptr;
    static ComPtr<ID3D12DeviceFactory> deviceFactory;

    struct D3D12Destroyer
    {
        ~D3D12Destroyer()
        {
            deviceFactory.Reset();

            if (lib_d3d12)
                FreeLibrary(lib_d3d12);

            if (lib_dxgi)
                FreeLibrary(lib_dxgi);
        }
    } d3d11_destroyer;

    class D3D12Device final : public RHIDevice
    {
        friend class D3D12CommandBuffer;

    public:
        D3D12Device(D3D12Adapter* adapter, const RHIDeviceDesc& desc);
        ~D3D12Device() override;

        bool WaitIdle() override;
        uint64_t CommitFrame() override;

        RHIBufferRef CreateBufferCore(const RHIBufferDesc& desc, RHINativeHandle nativeHandle, const void* initialData) override;
        RHITextureRef CreateTextureCore(const RHITextureDesc& desc, const RHITextureData* initialData) override;
        RHITextureRef CreateTextureFromNativeHandleCore(RHINativeHandle handle, const RHITextureDesc& desc) override;
        RHISamplerRef CreateSamplerCore(const RHISamplerDesc& desc) override;

        RHIShaderModuleRef CreateShaderModuleCore(const RHIShaderModuleDesc& desc) override;
        RHIComputePipelineRef CreateComputePipelineCore(const RHIComputePipelineDesc& desc) override;
        RHIRenderPipelineRef CreateRenderPipelineCore(const RHIRenderPipelineDesc& desc) override;
        RHIQueryHeapRef CreateQueryHeapCore(const RHIQueryHeapDesc& desc) override;
        //RHISwapChainRef CreateSwapChainCore(RHISurface* surface, const RHISwapChainDesc& desc) override;
        //void UpdateSwapChain(D3D12SwapChain* swapChain);

        void WriteShadingRateValue(RHIShadingRate rate, void* dest) const override;

        RHICommandBuffer* BeginCommandBuffer(RHIQueueType queueType, std::string_view label = "") override;

        //uint32_t AllocateBindlessResource(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        //uint32_t AllocateBindlessSampler(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        //void FreeBindlessResource(uint32_t index);
        //void FreeBindlessSampler(uint32_t index);

        void DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation = nullptr);
        void ProcessDeletionQueue(bool force);
        void OnDeviceRemoved();

        bool QueryFeatureSupport(RHIFeature feature) override;
        RHIPixelFormatSupport QueryPixelFormatSupport(PixelFormat format) override;
        bool QueryVertexFormatSupport(RHIVertexAttributeFormat format);
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;

        RHIBackend GetBackend() const override { return RHIBackend::D3D12; }
        RHIAdapter* GetAdapter() const override;
        auto GetHandle() const noexcept { return device; }
        ID3D12CommandQueue* GetD3D12GraphicsQueue() const noexcept { return queues[ecast(RHIQueueType::Graphics)].handle; }
        D3D12Queue& GetGraphicsQueue() { return queues[ecast(RHIQueueType::Graphics)]; }
        D3D12Queue& GetComputeQueue() { return queues[ecast(RHIQueueType::Compute)]; }
        D3D12Queue& GetCopyQueue() { return queues[ecast(RHIQueueType::Copy)]; }
        D3D12Queue& GetVideoDecode() { return queues[ecast(RHIQueueType::VideoDecode)]; }

    private:
        bool shuttingDown{ false };
        D3D12Adapter* _adapter;

        ID3D12Device5* device = nullptr;
        ID3D12Device8* device8 = nullptr;
        ComPtr<ID3D12VideoDevice> videoDevice;
        ComPtr<ID3D12DeviceConfiguration> deviceConfiguration;

        ComPtr<D3D12MA::Allocator> allocator;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        ComPtr<D3D12MA::Pool> umaPool;

        ID3D12Fence* deviceRemovedFence = nullptr;
        HANDLE deviceRemovedWaitHandle = nullptr;
#endif
        DWORD callbackCookie{};
        std::mutex onDeviceRemovedMutex;
        bool deviceRemoved = false;
        CD3DX12FeatureSupport d3dFeatures;
        bool enhancedBarriersSupported = false;

        D3D12Queue queues[ecast(RHIQueueType::Count)];

        struct CopyAllocator final
        {
            D3D12Device* device = nullptr;
            // Separate copy queue to reduce interference with main copy queue.
            ID3D12CommandQueue* queue = nullptr;
            std::mutex locker;
            std::vector<D3D12UploadContext> freeList;

            void Init(D3D12Device* device);
            void Shutdown();
            D3D12UploadContext Allocate(uint64_t size);
            void Submit(D3D12UploadContext context);
        };
        mutable CopyAllocator copyAllocator;

        ID3D12CommandSignature* dispatchIndirectCommandSignature = nullptr;
        ID3D12CommandSignature* drawIndirectCommandSignature = nullptr;
        ID3D12CommandSignature* drawIndexedIndirectCommandSignature = nullptr;
        ID3D12CommandSignature* dispatchMeshIndirectCommandSignature = nullptr;

        std::vector<std::unique_ptr<D3D12CommandBuffer>> commandBuffers;
        uint32_t cmdBuffersCount = 0;
        std::mutex cmdBuffersLocker;

    public:
        D3D12DescriptorAllocator renderTargetViewHeap;
        D3D12DescriptorAllocator depthStencilViewHeap;
        D3D12DescriptorAllocator shaderResourceViewHeap;
        D3D12DescriptorAllocator samplerHeap;

    private:
        // Bindless
        //static constexpr uint32_t kBindlessResourceCapacity = 500000;
        //static constexpr uint32_t kBindlessSamplerCapacity = 256;

        // Deletion queue objects
        std::mutex destroyMutex;
        std::deque<std::pair<D3D12MA::Allocation*, uint64_t>> deferredAllocations;
        std::deque<std::pair<IUnknown*, uint64_t>> deferredReleases;
        //std::vector<uint32_t> freeBindlessResources;
        //std::vector<uint32_t> freeBindlessSamplers;
        //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessResources;
        //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessSamplers;
    };

    class D3D12Adapter final : public RHIAdapter
    {
    public:
        D3D12Factory* factory;
        ComPtr<IDXGIAdapter1> handle;

        RHIDeviceRef CreateDevice(const RHIDeviceDesc& desc) override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
        IDXGIAdapter1* GetHandle() const noexcept { return handle.Get(); }
    };

    class D3D12Factory final : public RHIFactory
    {
    public:
        RHIValidationMode validationMode = RHIValidationMode::Disabled;
        ComPtr<IDXGIFactory4> handle = nullptr;
        bool tearingSupported = false;

        static bool IsSupported();

        D3D12Factory(const RHIFactoryDesc& desc);
        ~D3D12Factory() override;

        RHIBackend GetBackend() const override { return RHIBackend::D3D12; }

        RHISurfaceRef CreateSurface(void* window, void* display) override;
        RHINativeHandle GetNativeHandle(RHINativeHandleType objectType) override;
    };

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    inline void HandleDeviceRemoved(PVOID context, BOOLEAN)
    {
        D3D12Device* removedDevice = (D3D12Device*)context;
        removedDevice->OnDeviceRemoved();
    }
#endif

    /* D3D12Buffer */
    D3D12Buffer::~D3D12Buffer()
    {
        device->DeferDestroy(handle, allocation);
        handle = nullptr;
        allocation = nullptr;
    }

    RHINativeHandle D3D12Buffer::GetNativeHandle(RHINativeHandleType type)
    {
        switch (type)
        {
            case RHINativeHandleType::D3D12_Resource:
                return RHINativeHandle(handle);
            case RHINativeHandleType::SharedHandle:
                return RHINativeHandle(sharedHandle);
            default:
                return nullptr;
        }
    }

    /* D3D12Texture */
    D3D12Texture::~D3D12Texture()
    {
        for (auto& it : RTVs)
        {
            device->renderTargetViewHeap.ReleaseDescriptor(it.second);
        }
        RTVs.clear();

        for (auto& it : DSVs)
        {
            device->depthStencilViewHeap.ReleaseDescriptor(it.second);
        }
        DSVs.clear();

        device->DeferDestroy(handle, allocation);
        handle = nullptr;
        allocation = nullptr;
    }

    RHINativeHandle D3D12Texture::GetNativeHandle(RHINativeHandleType type)
    {
        switch (type)
        {
            case RHINativeHandleType::D3D12_Resource:
                return RHINativeHandle(handle);
            case RHINativeHandleType::SharedHandle:
                return RHINativeHandle(sharedHandle);
            default:
                return nullptr;
        }
    }

    /* D3D12Sampler */
    D3D12Sampler::~D3D12Sampler()
    {}

    RHITextureViewRef D3D12Texture::CreateView(const RHITextureViewDesc& desc) const
    {
        SharedPtr<D3D12TextureView> textureView(new D3D12TextureView(this, desc));
        if (CheckBitsAny(usage, RHITextureUsage::RenderTarget))
        {
            const bool isDepthStencil = IsDepthStencilFormat(desc.format);
            if (isDepthStencil)
            {
                textureView->DSVFormat = ToDxgiDSVFormat(desc.format);
                textureView->DSV = GetDSV(desc, textureView->DSVFormat, false);
                textureView->DSVReadOnly = GetDSV(desc, textureView->DSVFormat, true);
            }
            else
            {
                textureView->RTVFormat = ToDxgiRTVFormat(desc.format);
                textureView->RTV = GetRTV(desc, textureView->RTVFormat);
            }
        }

        return textureView;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetRTV(const RHITextureViewDesc& desc, DXGI_FORMAT rtvFormat) const
    {
        size_t hash = 0;

        HashCombine(hash, (uint32_t)rtvFormat);
        HashCombine(hash, (uint32_t)desc.aspect);
        HashCombine(hash, desc.baseMipLevel);
        HashCombine(hash, desc.mipLevelCount);
        HashCombine(hash, desc.baseArrayLayer);
        HashCombine(hash, desc.arrayLayerCount);

        auto it = RTVs.find(hash);
        if (it == RTVs.end())
        {
            const bool isArray = depthOrArrayLayers > 1;
            const bool isMultiSample = sampleCount > RHITextureSampleCount::Count1;

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.Format = rtvFormat;
            switch (dimension)
            {
                case RHITextureDimension::Texture1D:
                    if (isArray)
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice = desc.baseMipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = desc.baseArrayLayer;
                        viewDesc.Texture1DArray.ArraySize = desc.arrayLayerCount;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                        viewDesc.Texture1D.MipSlice = desc.baseMipLevel;
                    }
                    break;
                case RHITextureDimension::Texture2D:
                    if (isMultiSample)
                    {
                        viewDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                        if (isArray)
                        {
                            viewDesc.Texture2DMSArray.FirstArraySlice = desc.baseArrayLayer;
                            viewDesc.Texture2DMSArray.ArraySize = desc.arrayLayerCount;
                        }
                    }
                    else
                    {
                        viewDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
                        if (isArray)
                        {
                            viewDesc.Texture2DArray.MipSlice = desc.baseMipLevel;
                            viewDesc.Texture2DArray.FirstArraySlice = desc.baseArrayLayer;
                            viewDesc.Texture2DArray.ArraySize = desc.arrayLayerCount;
                            viewDesc.Texture2DArray.PlaneSlice = GetPlaneSlice(viewDesc.Format, desc.aspect);
                        }
                        else
                        {
                            viewDesc.Texture2D.MipSlice = desc.baseMipLevel;
                            viewDesc.Texture2D.PlaneSlice = GetPlaneSlice(viewDesc.Format, desc.aspect);
                        }
                    }
                    break;
                case RHITextureDimension::Texture3D:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                    viewDesc.Texture3D.MipSlice = desc.baseMipLevel;
                    viewDesc.Texture3D.FirstWSlice = desc.baseArrayLayer;
                    viewDesc.Texture3D.WSize = depthOrArrayLayers; // Or desc.layerCount?
                    break;
                case RHITextureDimension::TextureCube:
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MipSlice = desc.baseMipLevel;
                    viewDesc.Texture2DArray.FirstArraySlice = desc.baseArrayLayer;
                    viewDesc.Texture2DArray.ArraySize = desc.arrayLayerCount;
                    viewDesc.Texture2DArray.PlaneSlice = GetPlaneSlice(viewDesc.Format, desc.aspect);
                    break;

                default:
                    ALIMER_ASSERT_FAIL("Invalid TextureView type");
                    return {};
            }

            DescriptorIndex descriptorIndex = device->renderTargetViewHeap.AllocateDescriptors();
            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->renderTargetViewHeap.GetCpuHandle(descriptorIndex);
            device->GetHandle()->CreateRenderTargetView(handle, &viewDesc, cpuHandle);

            RTVs[hash] = descriptorIndex;
            return cpuHandle;
        }

        return device->renderTargetViewHeap.GetCpuHandle(it->second);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetDSV(const RHITextureViewDesc& desc, DXGI_FORMAT dsvFormat, bool readOnly) const
    {
        size_t hash = 0;

        HashCombine(hash, (uint32_t)dsvFormat);
        HashCombine(hash, (uint32_t)desc.aspect);
        HashCombine(hash, desc.baseMipLevel);
        HashCombine(hash, desc.mipLevelCount);
        HashCombine(hash, desc.baseArrayLayer);
        HashCombine(hash, desc.arrayLayerCount);
        HashCombine(hash, readOnly);

        auto it = DSVs.find(hash);
        if (it != DSVs.end())
            return device->depthStencilViewHeap.GetCpuHandle(it->second);

        const bool isArray = depthOrArrayLayers > 1;
        const bool isMultiSample = sampleCount > RHITextureSampleCount::Count1;

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
        viewDesc.Format = dsvFormat;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        if (readOnly)
        {
            viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;

            if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
                viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
        }

        switch (dimension)
        {
            case RHITextureDimension::Texture1D:
                if (isArray)
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = desc.baseMipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = desc.baseArrayLayer;
                    viewDesc.Texture1DArray.ArraySize = desc.arrayLayerCount;
                }
                else
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = desc.baseMipLevel;
                }
                break;
            case RHITextureDimension::Texture2D:
                if (isMultiSample)
                {
                    if (isArray)
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                        viewDesc.Texture2DMSArray.FirstArraySlice = desc.baseArrayLayer;
                        viewDesc.Texture2DMSArray.ArraySize = desc.arrayLayerCount;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                    }
                }
                else
                {
                    if (isArray)
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = desc.baseMipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = desc.baseArrayLayer;
                        viewDesc.Texture2DArray.ArraySize = desc.arrayLayerCount;
                    }
                    else
                    {
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                        viewDesc.Texture2D.MipSlice = desc.baseMipLevel;
                    }
                }
                break;

            case RHITextureDimension::TextureCube:
                viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Texture2DArray.MipSlice = desc.baseMipLevel;
                viewDesc.Texture2DArray.FirstArraySlice = desc.baseArrayLayer;
                viewDesc.Texture2DArray.ArraySize = desc.arrayLayerCount;
                break;

            case RHITextureDimension::Texture3D:
                ALIMER_ASSERT_FAIL("Invalid TextureView type");
                return {};

            default:
                ALIMER_ASSERT_FAIL("Invalid TextureView type");
                return {};
        }

        DescriptorIndex descriptorIndex = device->depthStencilViewHeap.AllocateDescriptors();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->depthStencilViewHeap.GetCpuHandle(descriptorIndex);
        device->GetHandle()->CreateDepthStencilView(handle, &viewDesc, cpuHandle);

        DSVs[hash] = descriptorIndex;
        return cpuHandle;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC D3D12TextureView::GetSRVDescriptor() const
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = ToDxgiSRVFormat(format);
        srvDesc.Shader4ComponentMapping = ToD3D12Swizzle(swizzle);

        const RHITextureDimension dimension = texture->GetDimension();
        if (dimension == RHITextureDimension::Texture1D)
        {
            if (texture->GetArrayLayers() > 1)
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                srvDesc.Texture1DArray.MostDetailedMip = baseMipLevel;
                srvDesc.Texture1DArray.MipLevels = mipLevelCount;
                srvDesc.Texture1DArray.FirstArraySlice = baseMipLevel;
                srvDesc.Texture1DArray.ArraySize = arrayLayerCount;
                srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
            }
            else
            {
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                srvDesc.Texture1D.MostDetailedMip = baseMipLevel;
                srvDesc.Texture1D.MipLevels = mipLevelCount;
                srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
            }
        }
        else if (dimension == RHITextureDimension::Texture2D || dimension == RHITextureDimension::TextureCube)
        {
            if (texture->GetArrayLayers() > 1)
            {
                if (dimension == RHITextureDimension::TextureCube)
                {
                    if (texture->GetArrayLayers() > 6 && arrayLayerCount > 6)
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                        srvDesc.TextureCubeArray.First2DArrayFace = baseMipLevel;
                        srvDesc.TextureCubeArray.NumCubes = Min(texture->GetArrayLayers(), arrayLayerCount) / 6;
                        srvDesc.TextureCubeArray.MostDetailedMip = baseMipLevel;
                        srvDesc.TextureCubeArray.MipLevels = mipLevelCount;
                        srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
                    }
                    else
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                        srvDesc.TextureCube.MostDetailedMip = baseMipLevel;
                        srvDesc.TextureCube.MipLevels = mipLevelCount;
                        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
                    }
                }
                else
                {
                    if (texture->IsMultisampled())
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        srvDesc.Texture2DMSArray.FirstArraySlice = baseArrayLayer;
                        srvDesc.Texture2DMSArray.ArraySize = arrayLayerCount;
                    }
                    else
                    {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srvDesc.Texture2DArray.FirstArraySlice = baseArrayLayer;
                        srvDesc.Texture2DArray.ArraySize = arrayLayerCount;
                        srvDesc.Texture2DArray.MostDetailedMip = baseMipLevel;
                        srvDesc.Texture2DArray.MipLevels = mipLevelCount;
                        srvDesc.Texture2DArray.PlaneSlice = GetPlaneSlice(srvDesc.Format, aspect);
                        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                }
            }
            else
            {
                if (texture->IsMultisampled())
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Texture2D.MostDetailedMip = baseMipLevel;
                    srvDesc.Texture2D.MipLevels = mipLevelCount;
                    srvDesc.Texture2D.PlaneSlice = GetPlaneSlice(srvDesc.Format, aspect);
                    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                }
            }
        }
        else if (dimension == RHITextureDimension::Texture3D)
        {
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            srvDesc.Texture3D.MostDetailedMip = baseMipLevel;
            srvDesc.Texture3D.MipLevels = mipLevelCount;
        }

        return srvDesc;
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12TextureView::GetUAVDescriptor() const
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc{};
        viewDesc.Format = ToDxgiSRVFormat(format);
        return viewDesc;
    }

    void D3D12Texture::SetLabel(const char* label)
    {
        auto wName = ToUtf16(label);
        handle->SetName(wName.c_str());
        allocation->SetName(wName.c_str());
    }

    /* D3D12ShaderModule */
    D3D12ShaderModule::~D3D12ShaderModule()
    {
        free(pByteCode);
        bytecode = {};
    }

    /* D3D12ComputePipeline */
    D3D12ComputePipeline::~D3D12ComputePipeline()
    {
        device->DeferDestroy(handle);
        handle = nullptr;
    }

    void D3D12ComputePipeline::SetLabel(const char* label)
    {
        auto wName = ToUtf16(label);
        handle->SetName(wName.c_str());
    }

    /* D3D12RenderPipeline */
    D3D12RenderPipeline::~D3D12RenderPipeline()
    {
        device->DeferDestroy(handle);
        handle = nullptr;
    }

    void D3D12RenderPipeline::SetLabel(const char* label)
    {
        auto wName = ToUtf16(label);
        handle->SetName(wName.c_str());
    }

    /* D3D12QueryHeap */
    D3D12QueryHeap::~D3D12QueryHeap()
    {
        device->DeferDestroy(handle);
        handle = nullptr;
    }

    void D3D12QueryHeap::SetLabel(const char* label)
    {
        auto wName = ToUtf16(label);
        handle->SetName(wName.c_str());
    }

    /* D3D12CommandBuffer */
    D3D12CommandBuffer::D3D12CommandBuffer(D3D12Device* device_, RHIQueueType queueType_, uint32_t id_)
        : device(device_)
        , queueType(queueType_)
        , id(id_)
    {
        D3D12_COMMAND_LIST_TYPE d3dCommandListType = ToD3D12(queueType);

        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            ThrowIfFailed(
                device->device->CreateCommandAllocator(d3dCommandListType, PPV_ARGS(commandAllocators[i]))
            );
        }

        ThrowIfFailed(
            device->device->CreateCommandList1(0, d3dCommandListType, D3D12_COMMAND_LIST_FLAG_NONE, PPV_ARGS(commandList6))
        );
        commandList6->QueryInterface(&commandList7);
    }

    D3D12CommandBuffer::~D3D12CommandBuffer()
    {
        Clear();

        for (uint32_t i = 0; i < kNumFramesInFlight; ++i)
        {
            SafeRelease(commandAllocators[i]);
        }

        SafeRelease(commandList7);
        SafeRelease(commandList6);
    }

    void D3D12CommandBuffer::Clear()
    {
        numBarriersToCommit = 0;
        globalBarriers.clear();
        textureBarriers.clear();
        bufferBarriers.clear();
        presentSwapChains.clear();
    }

    void D3D12CommandBuffer::Begin(uint32_t frameIndex, std::string_view label)
    {
        RHICommandBuffer::Reset(frameIndex);
        waits.clear();
        hasPendingWaits.store(false);
        Clear();

        // Start the command list in a default state:
        ThrowIfFailed(commandAllocators[frameIndex]->Reset());
        ThrowIfFailed(commandList6->Reset(commandAllocators[frameIndex], nullptr));

        if (queueType != RHIQueueType::Copy)
        {
            ID3D12DescriptorHeap* heaps[2] = {
                device->shaderResourceViewHeap.GetShaderVisibleHeap(),
                device->samplerHeap.GetShaderVisibleHeap()
            };
            commandList6->SetDescriptorHeaps(2u, heaps);
        }

        if (queueType == RHIQueueType::Graphics)
        {
            for (uint32_t i = 0; i < kMaxVertexBuffers; ++i)
            {
                vboViews[i] = {};
            }

            D3D12_RECT scissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX + 1];
            for (size_t i = 0; i < std::size(scissorRects); ++i)
            {
                scissorRects[i].bottom = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].left = D3D12_VIEWPORT_BOUNDS_MIN;
                scissorRects[i].right = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].top = D3D12_VIEWPORT_BOUNDS_MIN;
            }
            commandList6->RSSetScissorRects((uint32_t)std::size(scissorRects), scissorRects);

            const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            commandList6->OMSetBlendFactor(blendFactor);
        }

        hasLabel = !label.empty();
        if (hasLabel)
        {
            PushDebugGroup(label);
            hasLabel = true;
        }
    }

    ID3D12CommandList* D3D12CommandBuffer::End()
    {
        for (auto& swapChain : presentSwapChains)
        {
            TextureBarrier(swapChain->backbufferTextures[swapChain->backBufferIndex], RHITextureLayout::Present);
        }
        CommitBarriers();

        if (hasLabel)
        {
            PopDebugGroup();
        }

        ThrowIfFailed(commandList6->Close());

        return commandList6;
    }

    void D3D12CommandBuffer::BufferBarrier(const D3D12Buffer* buffer, RHIBufferStates newState, bool commit)
    {
        if (buffer->immutableState || buffer->currentState == newState)
            return;

        if (device->d3dFeatures.EnhancedBarriersSupported())
        {
            D3D12BufferStateMapping mappingBefore = ConvertBufferState(buffer->currentState);
            D3D12BufferStateMapping mappingAfter = ConvertBufferState(newState);

            D3D12_BUFFER_BARRIER& barrier = bufferBarriers.emplace_back();
            barrier.SyncBefore = mappingBefore.sync;
            barrier.SyncAfter = mappingAfter.sync;
            barrier.AccessBefore = mappingBefore.access;
            barrier.AccessAfter = mappingAfter.access;
            barrier.pResource = buffer->handle;
            barrier.Offset = 0;
            barrier.Size = UINT64_MAX;

            buffer->currentState = newState;
            numBarriersToCommit++;
        }
        else
        {
            const D3D12_RESOURCE_STATES oldStateLegacy = ConvertBufferStateLegacy(buffer->currentState, queueType);
            const D3D12_RESOURCE_STATES newStateLegacy = ConvertBufferStateLegacy(newState, queueType);

            if (queueType == RHIQueueType::Compute)
            {
                ALIMER_ASSERT((oldStateLegacy & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldStateLegacy);
                ALIMER_ASSERT((newStateLegacy & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newStateLegacy);
            }

            if (oldStateLegacy != newStateLegacy)
            {
                ALIMER_ASSERT_MSG(numBarriersToCommit < kMaxBarrierCount, "Exceeded arbitrary limit on buffered barriers");

                D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriersToCommit++];
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = buffer->handle;
                barrier.Transition.Subresource = 0;
                barrier.Transition.StateBefore = oldStateLegacy;
                barrier.Transition.StateAfter = newStateLegacy;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                buffer->currentState = newState;
            }
            else if (newStateLegacy == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                InsertUAVBarrier(buffer, commit);
            }
        }

        buffer->currentState = newState;

        if (commit || numBarriersToCommit == kMaxBarrierCount)
            CommitBarriers();
    }

    void D3D12CommandBuffer::TextureBarrier(const D3D12Texture* resource, RHITextureLayout newLayout, uint32_t subresource, bool commit)
    {
        const uint32_t index = (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) ? 0 : subresource;
        const RHITextureLayout currentLayout = resource->subResourcesStates[index];
        if (currentLayout == newLayout)
            return;

        if (device->enhancedBarriersSupported)
        {
            D3D12TextureLayoutMapping mappingBefore = ConvertTextureLayout(currentLayout);
            D3D12TextureLayoutMapping mappingAfter = ConvertTextureLayout(newLayout);

            D3D12_TEXTURE_BARRIER& barrier = textureBarriers.emplace_back();
            barrier.SyncBefore = mappingBefore.sync;
            barrier.SyncAfter = mappingAfter.sync;
            barrier.AccessBefore = mappingBefore.access;
            barrier.AccessAfter = mappingAfter.access;
            barrier.LayoutBefore = mappingBefore.layout;
            barrier.LayoutAfter = mappingAfter.layout;
            barrier.pResource = resource->handle;
            if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                barrier.Subresources.IndexOrFirstMipLevel = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Subresources.NumMipLevels = 0;
                barrier.Subresources.FirstArraySlice = 0;
                barrier.Subresources.NumArraySlices = 0;
                barrier.Subresources.FirstPlane = 0;
                barrier.Subresources.NumPlanes = 0;
            }
            else
            {
                // TODO:
            }
            barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO Handle discard when we have transient resources
            ALIMER_ASSERT(barrier.LayoutBefore != barrier.LayoutAfter);

            resource->subResourcesStates[index] = newLayout;
            numBarriersToCommit++;
        }
        else
        {
            const D3D12_RESOURCE_STATES oldStateLegacy = ConvertTextureLayoutLegacy(currentLayout);
            const D3D12_RESOURCE_STATES newStateLegacy = ConvertTextureLayoutLegacy(newLayout);

            if (queueType == RHIQueueType::Compute)
            {
                ALIMER_ASSERT((oldStateLegacy & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldStateLegacy);
                ALIMER_ASSERT((newStateLegacy & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newStateLegacy);
            }

            if (oldStateLegacy != newStateLegacy)
            {
                ALIMER_ASSERT_MSG(numBarriersToCommit < kMaxBarrierCount, "Exceeded arbitrary limit on buffered barriers");

                D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriersToCommit++];
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = resource->handle;
                barrier.Transition.Subresource = subresource; // D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = oldStateLegacy;
                barrier.Transition.StateAfter = newStateLegacy;

                // Check to see if we already started the transition
                //if (NewState == Resource.m_TransitioningState)
                //{
                //    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                //    Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
                //}
                //else
                {
                    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                }

                resource->subResourcesStates[index] = newLayout;
            }
            else if (newStateLegacy == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                InsertUAVBarrier(resource, commit);
            }
        }

        if (commit || numBarriersToCommit == kMaxBarrierCount)
            CommitBarriers();
    }

    void D3D12CommandBuffer::InsertUAVBarrier(const D3D12Resource* resource, bool commit)
    {
        ALIMER_ASSERT_MSG(numBarriersToCommit < kMaxBarrierCount, "Exceeded arbitrary limit on buffered barriers");

        D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriersToCommit++];
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.UAV.pResource = resource->handle;

        if (commit || numBarriersToCommit == kMaxBarrierCount)
            CommitBarriers();
    }

    void D3D12CommandBuffer::CommitBarriers()
    {
        if (!globalBarriers.empty() || !textureBarriers.empty() || !bufferBarriers.empty())
        {
            uint32_t numBarrierGroups = 0;
            D3D12_BARRIER_GROUP barrierGroups[3] = {};

            if (!globalBarriers.empty())
            {
                barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_GLOBAL;
                barrierGroups[numBarrierGroups].NumBarriers = (UINT32)globalBarriers.size();
                barrierGroups[numBarrierGroups].pGlobalBarriers = globalBarriers.data();
                numBarrierGroups++;
            }

            if (!textureBarriers.empty())
            {
                barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_TEXTURE;
                barrierGroups[numBarrierGroups].NumBarriers = (UINT32)textureBarriers.size();
                barrierGroups[numBarrierGroups].pTextureBarriers = textureBarriers.data();
                numBarrierGroups++;
            }

            if (!bufferBarriers.empty())
            {
                barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_BUFFER;
                barrierGroups[numBarrierGroups].NumBarriers = (UINT32)bufferBarriers.size();
                barrierGroups[numBarrierGroups].pBufferBarriers = bufferBarriers.data();
                numBarrierGroups++;
            }

            commandList7->Barrier(numBarrierGroups, barrierGroups);

            globalBarriers.clear();
            textureBarriers.clear();
            bufferBarriers.clear();
        }
        else if (numBarriersToCommit > 0)
        {
            commandList6->ResourceBarrier(numBarriersToCommit, barriers);
        }

        numBarriersToCommit = 0;
    }

    void D3D12CommandBuffer::PushDebugGroup(std::string_view name)
    {
        if (PIXBeginEventOnCommandList != nullptr)
        {
            PIXBeginEventOnCommandList(commandList6, PIX_COLOR_DEFAULT, name.data());
        }
        else
        {
            auto wide = ToUtf16(name);
            PIXBeginEvent(commandList6, PIX_COLOR_DEFAULT, wide.c_str());
        }
    }

    void D3D12CommandBuffer::PopDebugGroup()
    {
        if (PIXEndEventOnCommandList != nullptr)
        {
            PIXEndEventOnCommandList(commandList6);
        }
        else
        {
            PIXEndEvent(commandList6);
        }
    }

    void D3D12CommandBuffer::InsertDebugMarker(std::string_view name)
    {
        if (PIXSetMarkerOnCommandList != nullptr)
        {
            PIXSetMarkerOnCommandList(commandList6, PIX_COLOR_DEFAULT, name.data());
        }
        else
        {
            auto wide = ToUtf16(name);
            PIXSetMarker(commandList6, PIX_COLOR_DEFAULT, wide.c_str());
        }
    }
    void D3D12CommandBuffer::BeginQuery(const RHIQueryHeap* heap, uint32_t index)
    {
        auto backendHeap = static_cast<const D3D12QueryHeap*>(heap);
    }

    void D3D12CommandBuffer::EndQuery(const RHIQueryHeap* heap, uint32_t index)
    {
        auto backendHeap = static_cast<const D3D12QueryHeap*>(heap);
    }

    void D3D12CommandBuffer::ResolveQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count, const RHIBuffer* destinationBuffer, uint64_t destinationOffset)
    {
        auto backendHeap = static_cast<const D3D12QueryHeap*>(heap);
        auto backendDestBuffer = static_cast<const D3D12Buffer*>(destinationBuffer);
    }

    void D3D12CommandBuffer::ResetQuery(const RHIQueryHeap* heap, uint32_t index, uint32_t count)
    {
        auto backendHeap = static_cast<const D3D12QueryHeap*>(heap);
    }

    RHITexture* D3D12CommandBuffer::AcquireSurfaceTexture(RHISurface* surface)
    {
        return nullptr;
    }

    RHIComputePassEncoder* D3D12CommandBuffer::BeginComputePassCore(const RHIComputePassDesc& desc)
    {
        return nullptr;
    }

    RHIRenderPassEncoder* D3D12CommandBuffer::BeginRenderPassCore(const RHIRenderPassDesc& desc)
    {
        return nullptr;
    }

    RHIDevice* D3D12CommandBuffer::GetDevice() const
    {
        return device;
    }

    RHINativeHandle D3D12CommandBuffer::GetNativeHandle(RHINativeHandleType type)
    {
        switch (type)
        {
            case RHINativeHandleType::D3D12_GraphicsCommandList:
                return RHINativeHandle(commandList6);

            case RHINativeHandleType::D3D12_CommandAllocator:
                return RHINativeHandle(commandAllocators[device->GetFrameIndex()]);

            default:
                return nullptr;
        }
    }

    /* D3D12Device */

    D3D12Device::D3D12Device(D3D12Adapter* adapter, const RHIDeviceDesc& desc)
        : _adapter(adapter)
    {
        {
#if TODO
            const DXGI_GPU_PREFERENCE gpuPreference = (desc.powerPreference == GPUPowerPreference::LowPower) ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;

            ComPtr<IDXGIFactory6> dxgiFactory6;
            const bool queryByPreference = SUCCEEDED(dxgiFactory4.As(&dxgiFactory6));
            auto NextAdapter = [&](uint32_t index, IDXGIAdapter1** ppAdapter)
                {
                    if (queryByPreference)
                    {
                        return dxgiFactory6->EnumAdapterByGpuPreference(index, gpuPreference, IID_PPV_ARGS(ppAdapter));
                    }
                    return dxgiFactory4->EnumAdapters1(index, ppAdapter);
                };

            for (uint32_t i = 0; NextAdapter(i, dxgiAdapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 adapterDesc;
                ThrowIfFailed(dxgiAdapter->GetDesc1(&adapterDesc));

                // Don't select the Basic Render Driver adapter.
                if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    continue;
                }

                if (deviceFactory != nullptr)
                {
                    if (SUCCEEDED(deviceFactory->CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
                    {
                        break;
                    }
                }
                else
                {
                    if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device))))
                    {
                        break;
                    }
                }
            }
#endif // TODO

            // Create the DX12 API device object.
            device->SetName(L"AlimerDevice");

            // Try to get higher device interface
            device->QueryInterface(PPV_ARGS(device8));

            // Init feature check (https://devblogs.microsoft.com/directx/introducing-a-new-api-for-checking-feature-support-in-direct3d-12/)
            ThrowIfFailed(d3dFeatures.Init(device));

            enhancedBarriersSupported = d3dFeatures.EnhancedBarriersSupported() == TRUE;

            const RHIValidationMode validationMode = adapter->factory->validationMode;
            if (validationMode != RHIValidationMode::Disabled)
            {
                // Configure debug device (if active).
                ComPtr<ID3D12InfoQueue> infoQueue;
                if (SUCCEEDED(device->QueryInterface(infoQueue.GetAddressOf())))
                {
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

                    std::vector<D3D12_MESSAGE_SEVERITY> enabledSeverities;
                    std::vector<D3D12_MESSAGE_ID> disabledMessages;

                    // These severities should be seen all the time
                    enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_CORRUPTION);
                    enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_ERROR);
                    enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_WARNING);
                    enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_MESSAGE);

                    if (validationMode == RHIValidationMode::Verbose)
                    {
                        // Verbose only filters
                        enabledSeverities.push_back(D3D12_MESSAGE_SEVERITY_INFO);
                    }

                    disabledMessages.push_back(D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_GPU_WRITTEN_READBACK_RESOURCE_MAPPED);

#if defined (ALIMER_DX12_USE_PIPELINE_LIBRARY)
                    disabledMessages.push_back(D3D12_MESSAGE_ID_LOADPIPELINE_NAMENOTFOUND);
                    disabledMessages.push_back(D3D12_MESSAGE_ID_STOREPIPELINE_DUPLICATENAME);
#endif

                    D3D12_INFO_QUEUE_FILTER filter = {};
                    filter.AllowList.NumSeverities = static_cast<UINT>(enabledSeverities.size());
                    filter.AllowList.pSeverityList = enabledSeverities.data();
                    filter.DenyList.NumIDs = static_cast<UINT>(disabledMessages.size());
                    filter.DenyList.pIDList = disabledMessages.data();

                    // Clear out the existing filters since we're taking full control of them
                    infoQueue->PushEmptyStorageFilter();

                    ThrowIfFailed(infoQueue->AddStorageFilterEntries(&filter));
                }

                ComPtr<ID3D12InfoQueue1> infoQueue1 = nullptr;
                if (SUCCEEDED(device->QueryInterface(infoQueue1.GetAddressOf())))
                {
                    infoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &callbackCookie);
                }
            }

            // Create fence to detect device removal
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            {
                ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&deviceRemovedFence)));

                HANDLE deviceRemovedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
                ThrowIfFailed(deviceRemovedFence->SetEventOnCompletion(UINT64_MAX, deviceRemovedEvent));

                RegisterWaitForSingleObject(
                    &deviceRemovedWaitHandle,
                    deviceRemovedEvent,
                    HandleDeviceRemoved,
                    this, // Pass the device as our context
                    INFINITE, // No timeout
                    0 // No flags
                );
            }
#endif

            if (SUCCEEDED(device->QueryInterface(PPV_ARGS(videoDevice))))
            {
                LOGI("D3D12: Video device supported");
            }

            if (deviceFactory != nullptr)
            {
                ThrowIfFailed(device->QueryInterface(PPV_ARGS(deviceConfiguration)));
            }

            // Create allocator
            D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
            allocatorDesc.pDevice = device;
            allocatorDesc.pAdapter = adapter->GetHandle();
            allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
            allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
            allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED;
            //allocatorDesc.PreferredBlockSize = VMA_PREFERRED_BLOCK_SIZE;

            if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &allocator)))
            {
                return;
            }

            // Create command queues
            for (uint8_t queueIndex = 0; queueIndex < ecast(RHIQueueType::Count); ++queueIndex)
            {
                RHIQueueType queueType = (RHIQueueType)queueIndex;
                if (queueType >= RHIQueueType::VideoDecode && videoDevice == nullptr)
                    continue;

                queues[queueIndex].device = this;
                queues[queueIndex].queueType = queueType;

                D3D12_COMMAND_QUEUE_DESC queueDesc{};
                queueDesc.Type = ToD3D12(queueType);
                queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
                queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                queueDesc.NodeMask = 0;
                ThrowIfFailed(
                    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queues[queueIndex].handle))
                );
                ThrowIfFailed(
                    device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&queues[queueIndex].fence))
                );

                switch (queueType)
                {
                    case RHIQueueType::Graphics:
                        queues[queueIndex].handle->SetName(L"Graphics Queue");
                        queues[queueIndex].fence->SetName(L"GraphicsQueue - Fence");
                        break;
                    case RHIQueueType::Compute:
                        queues[queueIndex].handle->SetName(L"Compute Queue");
                        queues[queueIndex].fence->SetName(L"ComputeQueue - Fence");
                        break;
                    case RHIQueueType::Copy:
                        queues[queueIndex].handle->SetName(L"CopyQueue");
                        queues[queueIndex].fence->SetName(L"CopyQueue - Fence");
                        break;
                    case RHIQueueType::VideoDecode:
                        queues[queueIndex].handle->SetName(L"VideoDecode");
                        queues[queueIndex].fence->SetName(L"VideoDecode - Fence");
                        break;
                }

                // Create frame-resident resources
                for (uint32_t frameIndex = 0; frameIndex < kNumFramesInFlight; ++frameIndex)
                {
                    ThrowIfFailed(
                        device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&queues[queueIndex].frameFences[frameIndex]))
                    );

#if defined(_DEBUG)
                    wchar_t fenceName[64];

                    switch (queueType)
                    {
                        case RHIQueueType::Graphics:
                            swprintf(fenceName, 64, L"GraphicsQueue - Frame Fence %u", frameIndex);
                            break;
                        case RHIQueueType::Compute:
                            swprintf(fenceName, 64, L"ComputeQueue - Frame Fence %u", frameIndex);
                            break;
                        case RHIQueueType::Copy:
                            swprintf(fenceName, 64, L"CopyQueue - Frame Fence %u", frameIndex);
                            break;
                        case RHIQueueType::VideoDecode:
                            swprintf(fenceName, 64, L"VideoDecode - Frame Fence %u", frameIndex);
                            break;
                    }

                    queues[queueIndex].frameFences[frameIndex]->SetName(fenceName);
#endif
                }
            }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            if (d3dFeatures.CacheCoherentUMA())
            {
                D3D12MA::POOL_DESC poolDesc = {};
                poolDesc.HeapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
                poolDesc.HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
                poolDesc.HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
                ThrowIfFailed(allocator->CreatePool(&poolDesc, &umaPool));
            }
#endif

            copyAllocator.Init(this);

            // Create command signatures
            {
                // DispatchIndirectCommand
                D3D12_INDIRECT_ARGUMENT_DESC dispatchArg{};
                dispatchArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
                D3D12_COMMAND_SIGNATURE_DESC cmdSignatureDesc = {};
                cmdSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
                cmdSignatureDesc.NumArgumentDescs = 1;
                cmdSignatureDesc.pArgumentDescs = &dispatchArg;
                ThrowIfFailed(device->CreateCommandSignature(&cmdSignatureDesc, nullptr, IID_PPV_ARGS(&dispatchIndirectCommandSignature)));

                // DrawIndirectCommand
                D3D12_INDIRECT_ARGUMENT_DESC drawInstancedArg{};
                drawInstancedArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
                cmdSignatureDesc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
                cmdSignatureDesc.NumArgumentDescs = 1;
                cmdSignatureDesc.pArgumentDescs = &drawInstancedArg;
                ThrowIfFailed(device->CreateCommandSignature(&cmdSignatureDesc, nullptr, IID_PPV_ARGS(&drawIndirectCommandSignature)));

                // DrawIndexedIndirectCommand
                D3D12_INDIRECT_ARGUMENT_DESC drawIndexedInstancedArg{};
                drawIndexedInstancedArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

                cmdSignatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
                cmdSignatureDesc.NumArgumentDescs = 1;
                cmdSignatureDesc.pArgumentDescs = &drawIndexedInstancedArg;
                ThrowIfFailed(device->CreateCommandSignature(&cmdSignatureDesc, nullptr, IID_PPV_ARGS(&drawIndexedIndirectCommandSignature)));

                if (d3dFeatures.MeshShaderTier() >= D3D12_MESH_SHADER_TIER_1)
                {
                    D3D12_INDIRECT_ARGUMENT_DESC dispatchMeshArg{};
                    dispatchMeshArg.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

                    cmdSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_MESH_ARGUMENTS);
                    cmdSignatureDesc.NumArgumentDescs = 1;
                    cmdSignatureDesc.pArgumentDescs = &dispatchMeshArg;
                    ThrowIfFailed(device->CreateCommandSignature(&cmdSignatureDesc, nullptr, IID_PPV_ARGS(&dispatchMeshIndirectCommandSignature)));
                }
            }

            // Init CPU descriptor allocators
            const uint32_t renderTargetViewHeapSize = 1024;
            const uint32_t depthStencilViewHeapSize = 256;

            // Maximum number of CBV/SRV/UAV descriptors in heap for Tier 1
            const uint32_t shaderResourceViewHeapSize = 1000000;

            // Maximum number of samplers descriptors in heap for Tier 1
            const uint32_t samplerHeapSize = 2048; // 2048 ->  Tier1 limit

            renderTargetViewHeap.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetViewHeapSize);
            depthStencilViewHeap.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthStencilViewHeapSize);

            // Shader visible descriptor heaps
            shaderResourceViewHeap.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceViewHeapSize);
            samplerHeap.Init(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerHeapSize);

            // Init capabilities.
            //DXGI_ADAPTER_DESC1 adapterDesc;
            //ThrowIfFailed(dxgiAdapter->GetDesc1(&adapterDesc));

            // Determine maximum supported feature level for this device
            constexpr D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
            const D3D_FEATURE_LEVEL maxSupportedFeatureLevel = d3dFeatures.MaxSupportedFeatureLevel();
            if (maxSupportedFeatureLevel < minFeatureLevel)
            {
                std::string majorLevel = std::to_string(minFeatureLevel >> 12);
                std::string minorLevel = std::to_string((minFeatureLevel >> 8) & 0xF);
                LOGF("The device doesn't support the minimum feature level required to run this engine (DX{}.{})", majorLevel, minorLevel);
                return;
            }

            if (d3dFeatures.HighestRootSignatureVersion() < D3D_ROOT_SIGNATURE_VERSION_1_1)
            {
                LOGF("Direct3D12: Root signature version 1.1 not supported!");
                return;
            }

            // Check the required shader model
            //const D3D_SHADER_MODEL highestShaderModel = d3dFeatures.HighestShaderModel();
            //if (highestShaderModel < D3D_SHADER_MODEL_6_5)
            //{
            //    LOGF("Direct3D12: The device does not support the minimum shader model required to run this sample (SM 6.5)");
            //    return;
            //}

            uint32_t MaxNonSamplerDescriptors = 0;
            uint32_t MaxSamplerDescriptors = 0;
            D3D12_FEATURE_DATA_D3D12_OPTIONS19 options19{};
            if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS19, &options19, sizeof(options19))))
            {
                if (d3dFeatures.ResourceBindingTier() == D3D12_RESOURCE_BINDING_TIER_1)
                {
                    MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
                }
                else if (d3dFeatures.ResourceBindingTier() == D3D12_RESOURCE_BINDING_TIER_2)
                {
                    MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
                }
                else
                {
                    MaxNonSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
                }
                MaxSamplerDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
            }
            else
            {
                MaxNonSamplerDescriptors = options19.MaxViewDescriptorHeapSize;
                MaxSamplerDescriptors = options19.MaxSamplerDescriptorHeapSizeWithStaticSamplers;
            }

#if TODO
            adapterProperties.deviceName = ToUtf8(adapterDesc.Description);
            adapterProperties.vendorID = adapterDesc.VendorId;
            adapterProperties.deviceID = adapterDesc.DeviceId;

            // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
            LARGE_INTEGER umdVersion;
            if (dxgiAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
            {
                uint64_t encodedVersion = umdVersion.QuadPart;
                std::ostringstream o;
                o << "D3D12 driver version ";
                uint16_t driverVersion[4] = {};

                for (size_t i = 0; i < 4; ++i) {
                    driverVersion[i] = (encodedVersion >> (48 - 16 * i)) & 0xFFFF;
                    o << driverVersion[i] << ".";
                }

                adapterProperties.driverDescription = o.str();
            }

            // Detect adapter type.
            if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                adapterProperties.adapterType = AdapterType::Cpu;
            }
            else
            {
                adapterProperties.adapterType = d3dFeatures.UMA() ? AdapterType::IntegratedGpu : AdapterType::DiscreteGpu;
            }

            static_assert(kLUIDSize == sizeof(adapterDesc.AdapterLuid));
            memcpy(adapterProperties.luid, &adapterDesc.AdapterLuid, kLUIDSize);

            adapterProperties.videoMemorySize = adapterDesc.DedicatedVideoMemory;
            adapterProperties.systemMemorySize = adapterDesc.DedicatedSystemMemory + adapterDesc.SharedSystemMemory;
#endif // 0


            ThrowIfFailed(GetGraphicsQueue().handle->GetTimestampFrequency(&_timestampFrequency));

            // Limits
           // https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
           // In DWORDS. Descriptor tables cost 1, Root constants cost 1, Root descriptors cost 2.
            static constexpr uint32_t kMaxRootSignatureSize = 64u;

            _limits.maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
            _limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            _limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            _limits.maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
            _limits.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            _limits.maxBindGroups = kMaxRootSignatureSize;

            //uploadBufferTextureRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
            //uploadBufferTextureSliceAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

            _limits.maxConstantBufferBindingSize = D3D12_REQ_IMMEDIATE_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            _limits.maxStorageBufferBindingSize = (1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1;
            _limits.minConstantBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
            _limits.minStorageBufferOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT;

            //adapterProperties.limits.maxPushConstantsSize = sizeof(uint32_t) * kMaxRootSignatureSize / 1;
            //const uint32_t maxPushDescriptors = kMaxRootSignatureSize / 2;
            _limits.maxBufferSize = D3D12_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM * 1024ull * 1024ull;

            // Slot values can be 0-15, inclusive:
            // https://docs.microsoft.com/en-ca/windows/win32/api/d3d12/ns-d3d12-d3d12_input_element_desc
            _limits.maxVertexBuffers = kMaxVertexBuffers;
            _limits.maxVertexAttributes = std::min<uint32_t>(D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, kMaxVertexAttributes);
            _limits.maxVertexBufferArrayStride = D3D12_SO_BUFFER_MAX_STRIDE_IN_BYTES;

            _limits.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
            _limits.maxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            //adapterProperties.limits.samplerMinLodBias = D3D12_MIP_LOD_BIAS_MIN;
            //adapterProperties.limits.samplerMaxLodBias = D3D12_MIP_LOD_BIAS_MAX;
            _limits.samplerMaxAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY;

            // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
            // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
            // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
            _limits.maxComputeWorkgroupStorageSize = 32768;

            _limits.maxComputeInvocationsPerWorkgroup = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
            _limits.maxComputeWorkgroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
            _limits.maxComputeWorkgroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
            _limits.maxComputeWorkgroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;
            // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
            _limits.maxComputeWorkgroupsPerDimension = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

            _limits.highestShaderModel = FromD3D12(d3dFeatures.HighestShaderModel());

            // ConservativeRasterization
            _limits.conservativeRasterizationTier = static_cast<RHIConservativeRasterizationTier>(d3dFeatures.ConservativeRasterizationTier());

            if (d3dFeatures.TiledResourcesTier() >= D3D12_TILED_RESOURCES_TIER_1)
            {
                //capabilities |= GraphicsDeviceCapability::SPARSE_BUFFER;
                //capabilities |= GraphicsDeviceCapability::SPARSE_TEXTURE2D;

                if (d3dFeatures.TiledResourcesTier() >= D3D12_TILED_RESOURCES_TIER_2)
                {
                    //capabilities |= GraphicsDeviceCapability::SPARSE_NULL_MAPPING;

                    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                    //adapter->samplerMinMax = true;

                    if (d3dFeatures.TiledResourcesTier() >= D3D12_TILED_RESOURCES_TIER_3)
                    {
                        //capabilities |= GraphicsDeviceCapability::SPARSE_TEXTURE3D;
                    }
                }
            }

            // VariableRateShading
            _limits.variableShadingRateTier = static_cast<RHIVariableRateShadingTier>(d3dFeatures.VariableShadingRateTier());
            if (d3dFeatures.VariableShadingRateTier() != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED)
            {
                _limits.variableShadingRateImageTileSize = d3dFeatures.ShadingRateImageTileSize();
                _limits.isAdditionalVariableShadingRatesSupported = d3dFeatures.AdditionalShadingRatesSupported() == TRUE;
            }

            // Raytracing
            _limits.rayTracingTier = FromD3D12(d3dFeatures.RaytracingTier());
            if (d3dFeatures.RaytracingTier() >= D3D12_RAYTRACING_TIER_1_0)
            {
                _limits.rayTracingShaderGroupIdentifierSize = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;
                _limits.rayTracingShaderTableAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
                _limits.rayTracingShaderTableMaxStride = UINT_MAX;
                _limits.rayTracingShaderRecursionMaxDepth = D3D12_RAYTRACING_MAX_DECLARABLE_TRACE_RECURSION_DEPTH;
                _limits.rayTracingMaxGeometryCount = (1 << 24) - 1;
                _limits.rayTracingScratchAlignment = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT;
            }

            // MeshShader
            _limits.meshShaderTier = FromD3D12(d3dFeatures.MeshShaderTier());

            if (d3dFeatures.IndependentFrontAndBackStencilRefMaskSupported() == TRUE)
            {
                LOGD("D3D12: IndependentFrontAndBackStencilRefMaskSupported supported");
            }

            if (d3dFeatures.DynamicDepthBiasSupported() == TRUE)
            {
                LOGD("D3D12: DynamicDepthBiasSupported supported");
            }

            if (d3dFeatures.GPUUploadHeapSupported() == TRUE)
            {
                LOGD("D3D12: GPUUploadHeapSupported supported");
            }
        }

        InitResources();

#if defined(ALIMER_D3D12_AGILITY_SDK)
        LOGI("D3D12 (Agility SDK): Initialized with success");
#else
        LOGI("D3D12: Initialized with success");
#endif
    }

    D3D12Device::~D3D12Device()
    {
        WaitIdle();
        shuttingDown = true;

        commandBuffers.clear();
        cmdBuffersCount = 0;
        staticSamplers.clear();

        ProcessDeletionQueue(true);
        _frameCount = 0;

        for (uint8_t i = 0; i < ecast(RHIQueueType::Count); ++i)
        {
            SafeRelease(queues[i].handle);
            SafeRelease(queues[i].fence);

            for (uint32_t frameIndex = 0; frameIndex < kNumFramesInFlight; ++frameIndex)
            {
                SafeRelease(queues[i].frameFences[frameIndex]);
            }
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        umaPool.Reset();
#endif

        // Shutdown copy allocator
        copyAllocator.Shutdown();

        // CPU Heaps
        renderTargetViewHeap.Shutdown();
        depthStencilViewHeap.Shutdown();
        shaderResourceViewHeap.Shutdown();
        samplerHeap.Shutdown();

        SafeRelease(dispatchIndirectCommandSignature);
        SafeRelease(drawIndirectCommandSignature);
        SafeRelease(drawIndexedIndirectCommandSignature);
        SafeRelease(dispatchMeshIndirectCommandSignature);
        //SafeRelease(nullDescriptorHeapCbvSrvUav);

        // Allocator.
        if (allocator != nullptr)
        {
            D3D12MA::TotalStatistics stats;
            allocator->CalculateStatistics(&stats);

            if (stats.Total.Stats.AllocationBytes > 0)
            {
                LOGI("Total device memory leaked: {} bytes.", stats.Total.Stats.AllocationBytes);
            }

            allocator.Reset();
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        std::ignore = UnregisterWait(deviceRemovedWaitHandle);
        SafeRelease(deviceRemovedFence);
#endif

        if (callbackCookie)
        {
            ComPtr<ID3D12InfoQueue1> infoQueue1 = nullptr;
            ThrowIfFailed(device->QueryInterface(infoQueue1.GetAddressOf()));
            infoQueue1->UnregisterMessageCallback(callbackCookie);
            callbackCookie = 0;
        }

        videoDevice.Reset();
        deviceConfiguration.Reset();
        SafeRelease(device8);
        const ULONG refCount = device->Release();
#if defined(_DEBUG)
        if (refCount)
        {
            LOGD("There are {} unreleased references left on the D3D device!", refCount);

            ID3D12DebugDevice* debugDevice = nullptr;
            if (SUCCEEDED(device->QueryInterface(&debugDevice)))
            {
                debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                debugDevice->Release();
            }
        }
#else
        (void)refCount; // avoid warning
#endif
    }

    bool D3D12Device::WaitIdle()
    {
        ComPtr<ID3D12Fence> fence;
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

        for (uint8_t i = 0; i < ecast(RHIQueueType::Count); ++i)
        {
            if (queues[i].handle == nullptr)
                continue;

            //queues[queue].WaitIdle();
            ThrowIfFailed(queues[i].handle->Signal(fence.Get(), 1));

            if (fence->GetCompletedValue() < 1)
            {
                ThrowIfFailed(fence->SetEventOnCompletion(1, nullptr));
            }

            ThrowIfFailed(fence->Signal(0));
        }

        ProcessDeletionQueue(true);
        return true;
    }

    uint64_t D3D12Device::CommitFrame()
    {
        HRESULT hr;

        // Submit current frame:
        {
            uint32_t cmd_last = cmdBuffersCount;
            cmdBuffersCount = 0;
            for (uint32_t cmd = 0; cmd < cmd_last; ++cmd)
            {
                D3D12CommandBuffer& commandBuffer = *commandBuffers[cmd].get();
                ID3D12CommandList* commandList = commandBuffer.End();

                D3D12Queue& queue = queues[ecast(commandBuffer.queueType)];
                queue.submitCommandLists.push_back(commandList);

                if (commandBuffer.hasPendingWaits.load() || !commandBuffer.waits.empty())
                {
                    for (auto& waitCommandBuffer : commandBuffer.waits)
                    {
                        // record wait for signal on a previous submit:
                        hr = queue.handle->Wait(
                            queues[ecast(waitCommandBuffer->queueType)].fence,
                            _frameCount * commandBuffers.size() + (uint64_t)waitCommandBuffer->id
                        );
                        ThrowIfFailed(hr);
                    }

                    if (!queue.submitCommandLists.empty())
                    {
                        queue.handle->ExecuteCommandLists(
                            (UINT)queue.submitCommandLists.size(),
                            queue.submitCommandLists.data()
                        );
                        queue.submitCommandLists.clear();
                    }

                    if (commandBuffer.hasPendingWaits.load())
                    {
                        hr = queue.handle->Signal(
                            queue.fence,
                            _frameCount * commandBuffers.size() + (uint64_t)commandBuffer.id
                        );
                        ThrowIfFailed(hr);
                    }
                }
            }

            // Mark the completion of queues for this frame:
            for (uint8_t index = 0; index < ecast(RHIQueueType::Count); ++index)
            {
                D3D12Queue& queue = queues[index];

                if (!queue.submitCommandLists.empty())
                {
                    queue.handle->ExecuteCommandLists(
                        (UINT)queue.submitCommandLists.size(),
                        queue.submitCommandLists.data()
                    );
                    queue.submitCommandLists.clear();
                }

                hr = queue.handle->Signal(queue.frameFences[_frameIndex], 1);
                assert(SUCCEEDED(hr));
            }

            for (uint32_t cmd = 0; cmd < cmd_last; ++cmd)
            {
                D3D12CommandBuffer& commandBuffer = *commandBuffers[cmd].get();
                for (auto& swapChain : commandBuffer.presentSwapChains)
                {
                    hr = swapChain->handle->Present(swapChain->syncInterval, swapChain->presentFlags);

                    // If the device was reset we must completely reinitialize the renderer.
                    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
                    {
#ifdef _DEBUG
                        char buff[64] = {};
                        sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
                            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? device->GetDeviceRemovedReason() : hr));
                        OutputDebugStringA(buff);
#endif

                        // Handle device lost
                        OnDeviceRemoved();
                    }
                }
            }
        }

        // From here, we begin a new frame, this affects GetBufferIndex()!
        _frameCount++;
        _frameIndex = _frameCount % kNumFramesInFlight;

        // Initiate stalling CPU when GPU is not yet finished with next frame:
        for (uint8_t queue = 0; queue < ecast(RHIQueueType::Count); ++queue)
        {
            if (_frameCount >= kNumFramesInFlight &&
                queues[queue].frameFences[_frameIndex]->GetCompletedValue() < 1)
            {
                // NULL event handle will simply wait immediately:
                // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-seteventoncompletion#remarks
                ThrowIfFailed(queues[queue].frameFences[_frameIndex]->SetEventOnCompletion(1, nullptr));
            }
            ThrowIfFailed(queues[queue].frameFences[_frameIndex]->Signal(0));
        }

        ProcessDeletionQueue(false);

        return _frameCount;
    }

    RHIBufferRef D3D12Device::CreateBufferCore(const RHIBufferDesc& desc, RHINativeHandle nativeHandle, const void* initialData)
    {
        SharedPtr<D3D12Buffer> buffer(new D3D12Buffer(this, desc));

        if (nativeHandle)
        {
            buffer->handle = static_cast<ID3D12Resource*>(nativeHandle);

            if (desc.label)
            {
                buffer->SetLabel(desc.label);
            }

            return buffer;
        }

        uint64_t alignedSize = desc.size;
        if (CheckBitsAny(desc.usage, RHIBufferUsage::Constant))
        {
            alignedSize = AlignUp<uint64_t>(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        }

        D3D12_RESOURCE_DESC1 resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.Width = alignedSize;
        resourceDesc.Height = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.Alignment = 0;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.SamplerFeedbackMipRegion = {};

        if (CheckBitsAny(desc.usage, RHIBufferUsage::ShaderWrite))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        if (!CheckBitsAny(desc.usage, RHIBufferUsage::ShaderRead)
            && !CheckBitsAny(desc.usage, RHIBufferUsage::RayTracing))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }

        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;

        // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
        // Buffers may only use D3D12_BARRIER_LAYOUT_UNDEFINED as an initial layout.
        D3D12_BARRIER_LAYOUT initialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
        D3D12_RESOURCE_STATES initialStateLegacy = D3D12_RESOURCE_STATE_COMMON;

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        if (desc.memoryType == RHIMemoryType::Readback)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
            initialStateLegacy = D3D12_RESOURCE_STATE_COPY_DEST;
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

            buffer->immutableState = true;
        }
        else if (desc.memoryType == RHIMemoryType::Upload)
        {
            allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
            initialStateLegacy = D3D12_RESOURCE_STATE_GENERIC_READ;

            buffer->immutableState = true;
        }
        else
        {
            buffer->immutableState = false;
        }

        // TODO
        buffer->currentState = RHIBufferStates::Undefined;

        HRESULT hr = E_FAIL;
        const bool isSparse = false;
        if (isSparse)
            //if (CheckBitsAny(desc.usage, RHIBufferUsage::Sparse))
        {
            hr = device->CreateReservedResource(
                (D3D12_RESOURCE_DESC*)&resourceDesc,
                initialStateLegacy,
                nullptr,
                IID_PPV_ARGS(&buffer->handle)
            );
            //buffer->sparsePageSize = D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
        }
        else
        {
            if (enhancedBarriersSupported)
            {
                hr = allocator->CreateResource3(
                    &allocationDesc,
                    &resourceDesc,
                    initialLayout,
                    nullptr,
                    0, nullptr,
                    &buffer->allocation,
                    IID_PPV_ARGS(&buffer->handle)
                );
            }
            else
            {
                hr = allocator->CreateResource2(
                    &allocationDesc,
                    &resourceDesc,
                    initialStateLegacy,
                    nullptr,
                    &buffer->allocation,
                    IID_PPV_ARGS(&buffer->handle)
                );
            }
        }

        if (FAILED(hr))
        {
            return nullptr;
        }

        if (desc.label)
        {
            buffer->SetLabel(desc.label);
        }

        if (enhancedBarriersSupported)
        {
            device8->GetCopyableFootprints1(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &buffer->allocatedSize);
        }
        else
        {
            device->GetCopyableFootprints((D3D12_RESOURCE_DESC*)&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &buffer->allocatedSize);
        }

        buffer->deviceAddress = buffer->handle->GetGPUVirtualAddress();

        if (desc.memoryType == RHIMemoryType::Readback)
        {
            ThrowIfFailed(buffer->handle->Map(0, nullptr, &buffer->pMappedData));
        }
        else if (desc.memoryType == RHIMemoryType::Upload)
        {
            D3D12_RANGE readRange = {};
            ThrowIfFailed(buffer->handle->Map(0, &readRange, &buffer->pMappedData));
        }

        // Issue data copy on request
        if (initialData != nullptr)
        {
            if (desc.memoryType == RHIMemoryType::Upload)
            {
                std::memcpy(buffer->pMappedData, initialData, desc.size);
            }
            else
            {
                D3D12UploadContext context = copyAllocator.Allocate(alignedSize);

                std::memcpy(context.uploadBufferData, initialData, desc.size);

                context.commandList->CopyBufferRegion(
                    buffer->handle,
                    0,
                    context.uploadBuffer->handle,
                    0,
                    desc.size
                );

                copyAllocator.Submit(context);
            }
        }

        return buffer;
    }

    RHITextureRef D3D12Device::CreateTextureCore(const RHITextureDesc& desc, const RHITextureData* initialData)
    {
        SharedPtr<D3D12Texture> texture(new D3D12Texture(this, desc));
        texture->dxgiFormat = (DXGI_FORMAT)ToDxgiFormat(desc.format);

        const bool isDepthStencil = IsDepthStencilFormat(desc.format);

        if (isDepthStencil && CheckBitsAny(desc.usage, RHITextureUsage::ShaderReadWrite))
        {
            texture->dxgiFormat = GetTypelessFormatFromDepthFormat(desc.format);
        }

        const uint32_t arraySizeMultiplier = (desc.dimension == RHITextureDimension::TextureCube) ? 6 : 1;
        D3D12_RESOURCE_DESC1 resourceDesc{};
        switch (desc.dimension)
        {
            case RHITextureDimension::Texture1D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                resourceDesc.Width = desc.width;
                resourceDesc.Height = 1u;
                resourceDesc.DepthOrArraySize = (UINT16)desc.depthOrArrayLayers;
                break;

            case RHITextureDimension::Texture2D:
            case RHITextureDimension::TextureCube:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                resourceDesc.Width = desc.width;
                resourceDesc.Height = desc.height;
                resourceDesc.DepthOrArraySize = (UINT16)(desc.depthOrArrayLayers * arraySizeMultiplier);
                break;

            case RHITextureDimension::Texture3D:
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                resourceDesc.Width = desc.width;
                resourceDesc.Height = desc.height;
                resourceDesc.DepthOrArraySize = (UINT16)desc.depthOrArrayLayers;
                break;
        }
        resourceDesc.MipLevels = (UINT16)desc.mipLevelCount;
        resourceDesc.Format = texture->dxgiFormat;
        resourceDesc.SampleDesc.Count = ToD3D12(desc.sampleCount);
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.SamplerFeedbackMipRegion = {};

        if (CheckBitsAny(desc.usage, RHITextureUsage::RenderTarget))
        {
            if (isDepthStencil)
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                if (!CheckBitsAny(desc.usage, RHITextureUsage::ShaderRead))
                {
                    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                }
            }
            else
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }
        }

        if (CheckBitsAny(desc.usage, RHITextureUsage::ShaderWrite))
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        RHITextureLayout initialLayout = RHITextureLayout::Undefined;
        if (initialData == nullptr)
        {
            if (CheckBitsAny(desc.usage, RHITextureUsage::RenderTarget))
            {
                if (isDepthStencil)
                {
                    initialLayout = RHITextureLayout::DepthWrite;
                }
                else
                {
                    initialLayout = RHITextureLayout::RenderTarget;
                }
            }
            else if (CheckBitsAny(desc.usage, RHITextureUsage::ShaderWrite))
            {
                initialLayout = RHITextureLayout::UnorderedAccess;
            }
            else if (CheckBitsAny(desc.usage, RHITextureUsage::ShaderRead))
            {
                initialLayout = RHITextureLayout::ShaderResource;
            }
        }

        D3D12_CLEAR_VALUE clearValue = {};
        D3D12_CLEAR_VALUE* pClearValue = nullptr;

        if (CheckBitsAny(desc.usage, RHITextureUsage::RenderTarget))
        {
            clearValue.Format = resourceDesc.Format;
            if (isDepthStencil)
            {
                clearValue.DepthStencil.Depth = 0.0f; // Infinite reverse Z
            }
            pClearValue = &clearValue;
        }

        // If shader read/write and depth format, set to typeless
        if (IsDepthFormat(desc.format) && CheckBitsAny(desc.usage, RHITextureUsage::ShaderReadWrite))
        {
            pClearValue = nullptr;
        }

        texture->allocatedSize = 0;
        texture->numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
        texture->subResourcesStates.resize(texture->numSubResources);
        texture->footPrints.resize(texture->numSubResources);
        texture->rowSizesInBytes.resize(texture->footPrints.size());
        texture->numRows.resize(texture->footPrints.size());
        if (enhancedBarriersSupported)
        {
            device8->GetCopyableFootprints1(
                &resourceDesc,
                0,
                (UINT)texture->footPrints.size(),
                0,
                texture->footPrints.data(),
                texture->numRows.data(),
                texture->rowSizesInBytes.data(),
                &texture->allocatedSize
            );
        }
        else
        {
            device->GetCopyableFootprints(
                (D3D12_RESOURCE_DESC*)&resourceDesc,
                0,
                (UINT)texture->footPrints.size(),
                0,
                texture->footPrints.data(),
                texture->numRows.data(),
                texture->rowSizesInBytes.data(),
                &texture->allocatedSize
            );
        }

        for (uint32_t i = 0; i < texture->numSubResources; i++)
        {
            texture->subResourcesStates[i] = initialLayout;
        }

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

#if TEXTURE_STAGING
        // Staging textures
        if (desc.memoryType == MemoryType::Upload ||
            desc.memoryType == MemoryType::Readback)
        {
            pClearValue = nullptr;

            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = texture->allocatedSize;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (desc.memoryType == MemoryType::Readback)
            {
                allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
                resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
            }
            else
            {
                allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
                resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
            }
        }
        else
#endif // TEXTURE_STAGING
            if (CheckBitsAny(desc.usage, RHITextureUsage::Shared))
            {
                // Dedicated memory
                allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;

                // What about D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER?
                allocationDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_SHARED;
            }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        if (initialData != nullptr &&
            allocationDesc.HeapType == D3D12_HEAP_TYPE_DEFAULT &&
            d3dFeatures.CacheCoherentUMA() == TRUE
            )
        {
            // UMA: custom pool is like DEFAULT heap + CPU Write Combine
            // It will be used with WriteToSubresource to avoid GPU copy from UPLOAD to DEAFULT
            allocationDesc.CustomPool = umaPool.Get();
        }
#endif
        HRESULT hr = E_FAIL;
        if (enhancedBarriersSupported)
        {
            D3D12TextureLayoutMapping textureLayout = ConvertTextureLayout(initialLayout);

            hr = allocator->CreateResource3(
                &allocationDesc,
                &resourceDesc,
                textureLayout.layout,
                nullptr,
                0, nullptr,
                &texture->allocation,
                IID_PPV_ARGS(&texture->handle)
            );
        }
        else
        {
            D3D12_RESOURCE_STATES resourceState = ConvertTextureLayoutLegacy(initialLayout);

            hr = allocator->CreateResource2(
                &allocationDesc,
                &resourceDesc,
                resourceState,
                pClearValue,
                &texture->allocation,
                IID_PPV_ARGS(&texture->handle)
            );
        }

        if (FAILED(hr))
        {
            LOGE("D3D12: Failed to create texture");
            return nullptr;
        }

        if (desc.label)
        {
            texture->SetLabel(desc.label);
        }

#if TEXTURE_STAGING
        if (desc.memoryType == MemoryType::Readback)
        {
            ThrowIfFailed(texture->handle->Map(0, nullptr, &texture->pMappedData));
        }
        else if (desc.memoryType == MemoryType::Upload)
        {
            D3D12_RANGE readRange = {};
            ThrowIfFailed(texture->handle->Map(0, &readRange, &texture->pMappedData));
        }

        //if (texture->pMappedData != nullptr)
        //{
        //    texture->mapped_size = internal_state->total_size;
        //    internal_state->mapped_subresources.resize(internal_state->footprints.size());
        //    for (size_t i = 0; i < internal_state->footprints.size(); ++i)
        //    {
        //        internal_state->mapped_subresources[i].data_ptr = (uint8_t*)texture->mapped_data + internal_state->footprints[i].Offset;
        //        internal_state->mapped_subresources[i].row_pitch = internal_state->footprints[i].Footprint.RowPitch;
        //        internal_state->mapped_subresources[i].slice_pitch = internal_state->footprints[i].Footprint.RowPitch * internal_state->footprints[i].Footprint.Height;
        //    }
        //    texture->mapped_subresources = internal_state->mapped_subresources.data();
        //    texture->mapped_subresource_count = internal_state->mapped_subresources.size();
        //}  
#endif // TEXTURE_STAGING

        // Issue data copy on request:
        if (initialData != nullptr)
        {
            if (allocationDesc.CustomPool != nullptr &&
                allocationDesc.CustomPool == umaPool.Get())
            {
                // UMA direct texture write path:
                for (size_t i = 0; i < texture->footPrints.size(); ++i)
                {
                    const RHITextureData& data = initialData[i];

                    hr = texture->handle->WriteToSubresource(
                        (UINT)i,
                        nullptr,
                        data.pData,
                        data.rowPitch,
                        data.slicePitch
                    );
                    ThrowIfFailed(hr);
                }
            }
            else
            {
                std::vector<D3D12_SUBRESOURCE_DATA> subresourceData(texture->footPrints.size());
                for (size_t i = 0; i < texture->footPrints.size(); ++i)
                {
                    uint32_t rowPitch = initialData[i].rowPitch;
                    uint32_t slicePitch = initialData[i].slicePitch;
                    //GetSurfaceInfo(desc.format, texture->footprints[i].Footprint.Width, texture->footprints[i].Footprint.Height, &rowPitch, &slicePitch);

                    subresourceData[i].pData = initialData[i].pData;
                    subresourceData[i].RowPitch = rowPitch;
                    subresourceData[i].SlicePitch = slicePitch;
                }

                D3D12UploadContext context;
                void* pMappedData = nullptr;
#if TEXTURE_STAGING
                if (desc.memoryType == MemoryType::Upload)
                {
                    pMappedData = texture->pMappedData;
                }
                else
#endif // TEXTURE_STAGING

                {
                    context = copyAllocator.Allocate(texture->allocatedSize);
                    pMappedData = context.uploadBufferData;
                }

                for (size_t i = 0; i < texture->footPrints.size(); ++i)
                {
                    if (texture->rowSizesInBytes[i] > (SIZE_T)-1)
                        continue;

                    D3D12_MEMCPY_DEST DestData = {};
                    DestData.pData = (void*)((UINT64)pMappedData + texture->footPrints[i].Offset);
                    DestData.RowPitch = (SIZE_T)texture->footPrints[i].Footprint.RowPitch;
                    DestData.SlicePitch = (SIZE_T)texture->footPrints[i].Footprint.RowPitch * (SIZE_T)texture->numRows[i];

                    MemcpySubresource(&DestData,
                        &subresourceData[i],
                        (SIZE_T)texture->rowSizesInBytes[i],
                        texture->numRows[i],
                        texture->footPrints[i].Footprint.Depth
                    );

                    if (context.IsValid())
                    {
                        D3D12_TEXTURE_COPY_LOCATION dst = {};
                        dst.pResource = texture->handle;
                        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                        dst.SubresourceIndex = UINT(i);

                        D3D12_TEXTURE_COPY_LOCATION src = {};
                        src.pResource = context.uploadBuffer->handle;
                        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                        src.PlacedFootprint = texture->footPrints[i];

                        context.commandList->CopyTextureRegion(
                            &dst,
                            0,
                            0,
                            0,
                            &src,
                            nullptr
                        );
                    }
                }

                if (context.IsValid())
                {
                    copyAllocator.Submit(context);
                }
            }
        }

        if (CheckBitsAny(desc.usage, RHITextureUsage::Shared))
        {
            hr = device->CreateSharedHandle(
                texture->handle,
                nullptr,
                GENERIC_ALL,
                nullptr,
                &texture->sharedHandle);

            if (FAILED(hr))
            {
                LOGE("D3D12: Failed to create texture shared handle");
                return nullptr;
            }
        }

        return texture;
    }

    RHITextureRef D3D12Device::CreateTextureFromNativeHandleCore(RHINativeHandle handle, const RHITextureDesc& desc)
    {
        SharedPtr<D3D12Texture> texture(new D3D12Texture(this, desc));
        texture->dxgiFormat = (DXGI_FORMAT)ToDxgiFormat(desc.format);

        const bool isDepthStencil = IsDepthStencilFormat(desc.format);

        if (isDepthStencil && CheckBitsAny(desc.usage, RHITextureUsage::ShaderReadWrite))
        {
            texture->dxgiFormat = GetTypelessFormatFromDepthFormat(desc.format);
        }

        texture->handle = static_cast<ID3D12Resource*>(handle.pointer);

        D3D12_RESOURCE_DESC resourceDesc = texture->handle->GetDesc();
        texture->allocatedSize = 0;
        texture->numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
        texture->footPrints.resize(texture->numSubResources);
        texture->rowSizesInBytes.resize(texture->footPrints.size());
        texture->numRows.resize(texture->footPrints.size());
        device->GetCopyableFootprints(
            &resourceDesc,
            0,
            (UINT)texture->footPrints.size(),
            0,
            texture->footPrints.data(),
            texture->numRows.data(),
            texture->rowSizesInBytes.data(),
            &texture->allocatedSize
        );

        if (desc.label)
        {
            texture->SetLabel(desc.label);
        }

        return texture;
    }

    RHISamplerRef D3D12Device::CreateSamplerCore(const RHISamplerDesc& desc)
    {
        D3D12_SAMPLER_DESC d3d12Desc = ToD3D12SamplerDesc(desc);

        SharedPtr<D3D12Sampler> sampler(new D3D12Sampler());
        sampler->device = this;
        sampler->desc = desc;
        sampler->handle = d3d12Desc;

        return sampler;
    }

    RHIShaderModuleRef D3D12Device::CreateShaderModuleCore(const RHIShaderModuleDesc& desc)
    {
        SharedPtr<D3D12ShaderModule> module(new D3D12ShaderModule());
        module->device = this;
        module->pByteCode = (uint8_t*)malloc(desc.byteCodeSize);
        memcpy(module->pByteCode, desc.byteCode, desc.byteCodeSize);
        module->bytecode.BytecodeLength = desc.byteCodeSize;
        module->bytecode.pShaderBytecode = module->pByteCode;

        return module;
    }

    RHIComputePipelineRef D3D12Device::CreateComputePipelineCore(const RHIComputePipelineDesc& desc)
    {
        SharedPtr<D3D12ComputePipeline> pipeline(new D3D12ComputePipeline());
        pipeline->device = this;

        struct PSO_STREAM
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_CS CS;
        } stream;

        stream.pRootSignature = nullptr;
        stream.CS = StaticCast<D3D12ShaderModule>(desc.shader)->bytecode;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.pPipelineStateSubobjectStream = &stream;
        streamDesc.SizeInBytes = sizeof(stream);

        const HRESULT hr = device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipeline->handle));
        if (FAILED(hr))
        {
            return nullptr;
        }

        if (desc.label)
        {
            pipeline->SetLabel(desc.label);
        }

        return pipeline;
    }

    RHIRenderPipelineRef D3D12Device::CreateRenderPipelineCore(const RHIRenderPipelineDesc& desc)
    {
        SharedPtr<D3D12RenderPipeline> pipeline(new D3D12RenderPipeline());
        pipeline->device = this;

        // PipelineStream
        struct PSO_STREAM
        {
            struct PSO_STREAM1
            {
                CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
                CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
                CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE    IBStripCutValue;
                CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
                CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
                CD3DX12_PIPELINE_STATE_STREAM_HS                    HS;
                CD3DX12_PIPELINE_STATE_STREAM_DS                    DS;
                CD3DX12_PIPELINE_STATE_STREAM_GS                    GS;
                CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
                CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC            BlendState;
                CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1        DepthStencilState;
                CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
                CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER1           RasterizerState;
                CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
                CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
                CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK           SampleMask;
            } stream1 = {};

            struct PSO_STREAM2
            {
                CD3DX12_PIPELINE_STATE_STREAM_MS MS;
                CD3DX12_PIPELINE_STATE_STREAM_AS AS;
            } stream2 = {};
        } stream = {};

        stream.stream1.pRootSignature = nullptr;

        // Mesh Pipeline (D3DX12_MESH_SHADER_PIPELINE_STATE_DESC)
        if (desc.meshShader != nullptr)
        {
            stream.stream2.MS = StaticCast<D3D12ShaderModule>(desc.meshShader)->bytecode;

            if (desc.amplificationShader != nullptr)
            {
                stream.stream2.AS = StaticCast<D3D12ShaderModule>(desc.amplificationShader)->bytecode;
            }
        }
        else
        {
            stream.stream1.VS = StaticCast<D3D12ShaderModule>(desc.vertexShader)->bytecode;
        }

        if (desc.fragmentShader != nullptr)
        {
            stream.stream1.PS = StaticCast<D3D12ShaderModule>(desc.fragmentShader)->bytecode;
        }

        // InputLayout
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        D3D12_INPUT_LAYOUT_DESC inputLayout = {};
        if (desc.vertexBufferLayoutCount > 0)
        {
            for (uint32_t bufferIndex = 0; bufferIndex < desc.vertexBufferLayoutCount; ++bufferIndex)
            {
                const RHIVertexBufferLayout& vertexBufferLayout = desc.vertexBufferLayouts[bufferIndex];

                pipeline->numVertexBindings = Max(bufferIndex + 1, pipeline->numVertexBindings);
                pipeline->strides[bufferIndex] = vertexBufferLayout.stride;

                uint32_t currentOffset = 0;
                const bool computeStride = vertexBufferLayout.stride == 0;
                for (uint32_t attributeIndex = 0; attributeIndex < vertexBufferLayout.attributeCount; ++attributeIndex)
                {
                    const RHIVertexAttribute& attribute = vertexBufferLayout.attributes[attributeIndex];

                    // TODO
                    auto& element = inputElements.emplace_back();
                    element.SemanticName = "ATTRIBUTE";
                    element.SemanticIndex = 0;
                    element.Format = ToDxgiFormat(attribute.format);
                    element.InputSlot = bufferIndex;
                    element.AlignedByteOffset = attribute.offset;

                    if (vertexBufferLayout.stepMode == RHIVertexStepMode::Vertex)
                    {
                        element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                        element.InstanceDataStepRate = 0;
                    }
                    else
                    {
                        element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                        element.InstanceDataStepRate = 1;
                    }


                    const RHIVertexAttributeFormatInfo& formatInfo = GetVertexAttributeFormatInfo(attribute.format);
                    currentOffset += formatInfo.byteSize;

                    inputLayout.NumElements++;
                }

                // Compute stride from attributes
                if (computeStride)
                {
                    pipeline->strides[bufferIndex] = currentOffset;
                }
            }

            inputLayout.pInputElementDescs = inputElements.data();
            stream.stream1.InputLayout = inputLayout;
        }

        // Handle index strip
        if (desc.primitiveTopology != RHIPrimitiveTopology::TriangleStrip
            && desc.primitiveTopology != RHIPrimitiveTopology::LineStrip)
        {
            stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        }
        else
        {
            stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        }

        // Color Attachments + RTV
        D3D12_RT_FORMAT_ARRAY RTVFormats = {};
        RTVFormats.NumRenderTargets = 0;

        CD3DX12_BLEND_DESC blendState = {};
        blendState.AlphaToCoverageEnable = desc.blendState.alphaToCoverageEnable ? TRUE : FALSE;
        blendState.IndependentBlendEnable = desc.blendState.independentBlendEnable ? TRUE : FALSE;
        for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
        {
            if (desc.colorAttachmentFormats[i] == PixelFormat::Undefined)
                continue;

            const RHIRenderTargetBlendState& attachment = desc.blendState.renderTargets[i];

            blendState.RenderTarget[i].BlendEnable = BlendEnabled(&attachment) ? TRUE : FALSE;
            blendState.RenderTarget[i].LogicOpEnable = FALSE;
            blendState.RenderTarget[i].SrcBlend = D3D12Blend(attachment.srcColorBlendFactor);
            blendState.RenderTarget[i].DestBlend = D3D12Blend(attachment.destColorBlendFactor);
            blendState.RenderTarget[i].BlendOp = D3D12BlendOperation(attachment.colorBlendOp);
            blendState.RenderTarget[i].SrcBlendAlpha = D3D12AlphaBlend(attachment.srcAlphaBlendFactor);
            blendState.RenderTarget[i].DestBlendAlpha = D3D12AlphaBlend(attachment.destAlphaBlendFactor);
            blendState.RenderTarget[i].BlendOpAlpha = D3D12BlendOperation(attachment.alphaBlendOp);
            blendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
            blendState.RenderTarget[i].RenderTargetWriteMask = ToD3D12(attachment.colorWriteMask);

            // RTV
            RTVFormats.RTFormats[RTVFormats.NumRenderTargets] = (DXGI_FORMAT)ToDxgiFormat(desc.colorAttachmentFormats[i]);
            RTVFormats.NumRenderTargets++;
        }
        stream.stream1.RTVFormats = RTVFormats;
        stream.stream1.BlendState = blendState;

        // DepthStencilState + DSVFormat
        CD3DX12_DEPTH_STENCIL_DESC1 depthStencilState{};
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        if (desc.depthStencilFormat != PixelFormat::Undefined)
        {
            // Depth
            depthStencilState.DepthEnable =
                (desc.depthStencilState.depthCompare == RHICompareFunction::Always && !desc.depthStencilState.depthWriteEnabled) ? FALSE : TRUE;
            depthStencilState.DepthWriteMask = desc.depthStencilState.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilState.DepthFunc = ToD3D12(desc.depthStencilState.depthCompare);

            depthStencilState.StencilEnable = StencilTestEnabled(&desc.depthStencilState) ? TRUE : FALSE;
            depthStencilState.StencilReadMask = static_cast<UINT8>(desc.depthStencilState.stencilReadMask);
            depthStencilState.StencilWriteMask = static_cast<UINT8>(desc.depthStencilState.stencilWriteMask);

            depthStencilState.FrontFace = ToD3D12StencilOpDesc(desc.depthStencilState.frontFace);
            depthStencilState.BackFace = ToD3D12StencilOpDesc(desc.depthStencilState.backFace);

            if (d3dFeatures.DepthBoundsTestSupported() == TRUE)
            {
                depthStencilState.DepthBoundsTestEnable = desc.depthStencilState.depthBoundsTestEnable ? TRUE : FALSE;
            }
            else
            {
                depthStencilState.DepthBoundsTestEnable = FALSE;
            }
        }
        else
        {
            depthStencilState.DepthEnable = FALSE;
            depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            depthStencilState.StencilEnable = FALSE;
            depthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
            depthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
            depthStencilState.FrontFace = defaultStencilOp;
            depthStencilState.BackFace = defaultStencilOp;
            depthStencilState.DepthBoundsTestEnable = FALSE;
        }
        stream.stream1.DepthStencilState = depthStencilState;
        stream.stream1.DSVFormat = (DXGI_FORMAT)ToDxgiFormat(desc.depthStencilFormat);

        // RasterizerState
        CD3DX12_RASTERIZER_DESC1 rasterizerState{};
        rasterizerState.FillMode = ToD3D12(desc.rasterizerState.fillMode);
        rasterizerState.CullMode = ToD3D12(desc.rasterizerState.cullMode);
        rasterizerState.FrontCounterClockwise = (desc.rasterizerState.frontFace == RHIFrontFace::CounterClockwise) ? TRUE : FALSE;
        rasterizerState.DepthBias = desc.rasterizerState.depthBias;
        rasterizerState.DepthBiasClamp = desc.rasterizerState.depthBiasClamp;
        rasterizerState.SlopeScaledDepthBias = desc.rasterizerState.depthBiasSlopeScale;
        rasterizerState.DepthClipEnable = (desc.rasterizerState.depthClipMode == RHIDepthClipMode::Clip) ? TRUE : FALSE;
        rasterizerState.MultisampleEnable = desc.sampleCount > RHITextureSampleCount::Count1 ? TRUE : FALSE;
        rasterizerState.AntialiasedLineEnable = FALSE;
        rasterizerState.ForcedSampleCount = 0;
        rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        if (desc.rasterizerState.conservativeRasterEnable &&
            d3dFeatures.ConservativeRasterizationTier() != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED)
        {
            rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
        }
        stream.stream1.RasterizerState = rasterizerState;

        switch (desc.primitiveTopology)
        {
            case RHIPrimitiveTopology::PointList:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
                break;
            case RHIPrimitiveTopology::LineList:
            case RHIPrimitiveTopology::LineStrip:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
                break;
            case RHIPrimitiveTopology::TriangleList:
            case RHIPrimitiveTopology::TriangleStrip:
                stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                break;
        }

        // SampleDesc and SampleMask
        DXGI_SAMPLE_DESC sampleDesc = {};
        sampleDesc.Count = ToD3D12(desc.sampleCount);
        sampleDesc.Quality = 0;
        stream.stream1.SampleDesc = sampleDesc;
        stream.stream1.SampleMask = UINT_MAX;

        D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
        streamDesc.pPipelineStateSubobjectStream = &stream;
        streamDesc.SizeInBytes = sizeof(stream.stream1);
        if (d3dFeatures.MeshShaderTier() != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
        {
            streamDesc.SizeInBytes += sizeof(stream.stream2);
        }

        if (FAILED(device->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&pipeline->handle))))
        {
            return nullptr;
        }

        if (desc.label)
        {
            pipeline->SetLabel(desc.label);
        }

        pipeline->primitiveTopology = ToD3DPrimitiveTopology(desc.primitiveTopology);

        return pipeline;
    }

    RHIQueryHeapRef D3D12Device::CreateQueryHeapCore(const RHIQueryHeapDesc& desc)
    {
        D3D12_QUERY_HEAP_DESC d3dDesc = {};
        d3dDesc.Type = ToD3D12QueryHeapType(desc.type);
        d3dDesc.Count = desc.count;
        d3dDesc.NodeMask = 0;

        ID3D12QueryHeap* handle = nullptr;
        HRESULT hr = device->CreateQueryHeap(&d3dDesc, IID_PPV_ARGS(&handle));
        if (FAILED(hr))
        {
            return nullptr;
        }

        SharedPtr<D3D12QueryHeap> resource(new D3D12QueryHeap());
        resource->device = this;
        resource->desc = desc;
        resource->queryType = ToD3D12QueryType(desc.type);
        resource->handle = handle;
        resource->queryResultSize = GetQueryResultSize(desc.type);

        if (desc.label)
        {
            resource->SetLabel(desc.label);
        }

        return resource;
    }

    void D3D12Device::WriteShadingRateValue(RHIShadingRate rate, void* dest) const
    {
        D3D12_SHADING_RATE _rate = ToD3D12(rate);
        if (d3dFeatures.AdditionalShadingRatesSupported() == FALSE)
        {
            _rate = std::min(_rate, D3D12_SHADING_RATE_2X2);
        }
        *(uint8_t*)dest = _rate;
    }

    RHICommandBuffer* D3D12Device::BeginCommandBuffer(RHIQueueType queueType, std::string_view label)
    {
        cmdBuffersLocker.lock();
        uint32_t index = cmdBuffersCount++;
        if (index >= commandBuffers.size())
        {
            commandBuffers.push_back(std::make_unique<D3D12CommandBuffer>(this, queueType, index));
        }
        cmdBuffersLocker.unlock();

        commandBuffers[index]->Begin(_frameIndex, label);

        return commandBuffers[index].get();
    }

#if ALIMER_BINDLESS
    uint32_t D3D12Device::AllocateBindlessResource(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        destroyMutex.lock();
        if (!freeBindlessResources.empty())
        {
            uint32_t index = freeBindlessResources.back();
            freeBindlessResources.pop_back();
            destroyMutex.unlock();

            D3D12_CPU_DESCRIPTOR_HANDLE dstBindless = resourceDescriptorHeap.cpuStart;
            dstBindless.ptr += index * GetDescriptorHandleIncrementSize(DescriptorHeapType::CbvSrvUav);
            device->CopyDescriptorsSimple(1, dstBindless, handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            return index;
        }

        destroyMutex.unlock();
        return kInvalidBindlessIndex;
    }

    uint32_t D3D12Device::AllocateBindlessSampler(D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        destroyMutex.lock();
        if (!freeBindlessResources.empty())
        {
            uint32_t index = freeBindlessResources.back();
            freeBindlessResources.pop_back();
            destroyMutex.unlock();

            D3D12_CPU_DESCRIPTOR_HANDLE dstBindless = samplerDescriptorHeap.cpuStart;
            dstBindless.ptr += index * GetDescriptorHandleIncrementSize(DescriptorHeapType::Sampler);
            device->CopyDescriptorsSimple(1, dstBindless, handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
            return index;
        }

        destroyMutex.unlock();
        return kInvalidBindlessIndex;
    }

    void D3D12Device::FreeBindlessResource(uint32_t index)
    {
        if (index != kInvalidBindlessIndex)
        {
            destroyMutex.lock();
            destroyedBindlessResources.push_back(std::make_pair(index, frameCount));
            destroyMutex.unlock();
        }
    }

    void D3D12Device::FreeBindlessSampler(uint32_t index)
    {
        if (index != kInvalidBindlessIndex)
        {
            destroyMutex.lock();
            destroyedBindlessSamplers.push_back(std::make_pair(index, frameCount));
            destroyMutex.unlock();
        }
    }
#endif // ALIMER_BINDLESS

    void D3D12Device::DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation)
    {
        if (resource == nullptr)
        {
            return;
        }

        if (shuttingDown)
        {
            resource->Release();
            SafeRelease(allocation);
            return;
        }

        destroyMutex.lock();
        deferredReleases.push_back(std::make_pair(resource, _frameCount));
        if (allocation != nullptr)
        {
            deferredAllocations.push_back(std::make_pair(allocation, _frameCount));
        }
        destroyMutex.unlock();
    }

    void D3D12Device::ProcessDeletionQueue(bool force)
    {
        destroyMutex.lock();
        while (!deferredAllocations.empty())
        {
            if (force
                || (deferredAllocations.front().second + kNumFramesInFlight < _frameCount))
            {
                auto& item = deferredAllocations.front();
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
            if (force
                || (deferredReleases.front().second + kNumFramesInFlight < _frameCount))
            {
                auto& item = deferredReleases.front();
                deferredReleases.pop_front();
                item.first->Release();
            }
            else
            {
                break;
            }
        }

#if defined(ALIMER_RHI_BINDLESS)
        while (!destroyedBindlessResources.empty())
        {
            if (force
                || (destroyedBindlessResources.front().second + kNumFramesInFlight < _frameCount))
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
            if (force
                || (destroyedBindlessSamplers.front().second + kNumFramesInFlight < _frameCount))
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
#endif // defined(ALIMER_BINDLESS)

        destroyMutex.unlock();
    }

    void D3D12Device::OnDeviceRemoved()
    {
        std::lock_guard<std::mutex> lock(onDeviceRemovedMutex);

        if (deviceRemoved)
        {
            return;
        }
        deviceRemoved = true;

        ID3D12Device* removedDevice = device;

        const char* removedReasonString;
        HRESULT removedReason = removedDevice->GetDeviceRemovedReason();

        switch (removedReason)
        {
            case DXGI_ERROR_DEVICE_HUNG:
                removedReasonString = "DXGI_ERROR_DEVICE_HUNG";
                break;
            case DXGI_ERROR_DEVICE_REMOVED:
                removedReasonString = "DXGI_ERROR_DEVICE_REMOVED";
                break;
            case DXGI_ERROR_DEVICE_RESET:
                removedReasonString = "DXGI_ERROR_DEVICE_RESET";
                break;
            case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
                removedReasonString = "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
                break;
            case DXGI_ERROR_INVALID_CALL:
                removedReasonString = "DXGI_ERROR_INVALID_CALL";
                break;
            case DXGI_ERROR_ACCESS_DENIED:
                removedReasonString = "DXGI_ERROR_ACCESS_DENIED";
                break;
            default:
                removedReasonString = "Unknown DXGI error";
                break;
        }

#if defined(_DEBUG)
        //RHILOGTrace("Device was removed because of the following reason:\n");
        //const char* removedReasonString;

        // Perform app-specific device removed operation, such as logging or inspecting DRED output
        ComPtr<ID3D12DeviceRemovedExtendedData1> pDred;
        if (SUCCEEDED(removedDevice->QueryInterface(PPV_ARGS(pDred))))
        {
            D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT1 dredAutoBreadcrumbsOutput;
            HRESULT hr = pDred->GetAutoBreadcrumbsOutput1(&dredAutoBreadcrumbsOutput);
            if (SUCCEEDED(hr))
            {
                // TODO: Log DRED info -> to file?
            }
        }
#endif

        // TODO: Understand how to fix 'fmt::v11::fstring<>::fstring': call to immediate function is not a constant expression
        std::string message = "D3D12: Device was removed, cause: ";
        message += removedReasonString;
        LOGF("{}", message);
    }

    bool D3D12Device::QueryFeatureSupport(RHIFeature feature)
    {
        switch (feature)  // NOLINT(clang-diagnostic-switch-enum)
        {
            // Always supported features
            case RHIFeature::TimestampQuery:
            case RHIFeature::PipelineStatisticsQuery:
            case RHIFeature::TextureCompressionBC:
            case RHIFeature::IndirectFirstInstance:
            case RHIFeature::TessellationShader:
            case RHIFeature::SamplerMirrorOnce:
            case RHIFeature::SamplerBorder:
            case RHIFeature::DepthResolveMinMax:
            case RHIFeature::StencilResolveMinMax:
                //case RHIFeature::Predication:
                return true;

                // Always unsupported features
            case RHIFeature::TextureCompressionETC2:
            case RHIFeature::TextureCompressionASTC:
            case RHIFeature::TextureCompressionASTC_HDR:
                return false;

            case RHIFeature::ShaderFloat16:
                //const bool supportsDP4a = d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_4;
                return d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_2 && d3dFeatures.Native16BitShaderOpsSupported();

                //case RHIFeature::BGRA8UnormStorage:
                //{
                //    D3D12_FEATURE_DATA_FORMAT_SUPPORT bgra8unormFormatInfo = {};
                //    bgra8unormFormatInfo.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                //    HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &bgra8unormFormatInfo, sizeof(bgra8unormFormatInfo));
                //    if (SUCCEEDED(hr) &&
                //        (bgra8unormFormatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW))
                //    {
                //        return true;
                //    }
                //
                //    return false;
                //}

            case RHIFeature::GPUUploadHeapSupported:
                return (d3dFeatures.GPUUploadHeapSupported() == TRUE);

            case RHIFeature::CopyQueueTimestampQuery:
                return (d3dFeatures.CopyQueueTimestampQueriesSupported() == TRUE);

            case RHIFeature::DepthBoundsTest:
                return (d3dFeatures.DepthBoundsTestSupported() == TRUE);

            case RHIFeature::SamplerMinMax:
                if (d3dFeatures.TiledResourcesTier() >= D3D12_TILED_RESOURCES_TIER_2)
                {
                    // Tier 2 for tiled resources
                    // https://learn.microsoft.com/en-us/windows/win32/direct3d11/tiled-resources-texture-sampling-features
                }

                return (d3dFeatures.MaxSupportedFeatureLevel() >= D3D_FEATURE_LEVEL_11_1);

            case RHIFeature::CacheCoherentUMA:
                return (d3dFeatures.CacheCoherentUMA() == TRUE);

            case RHIFeature::ShaderOutputViewportIndex:
                return (d3dFeatures.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation() == TRUE);

            default:
                return false;
        }
    }

    RHIPixelFormatSupport D3D12Device::QueryPixelFormatSupport(PixelFormat format)
    {
        RHIPixelFormatSupport result = RHIPixelFormatSupport::None;
        const DXGI_FORMAT dxgiFormat = (DXGI_FORMAT)ToDxgiFormat(format);
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return result;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData = {};
        featureData.Format = dxgiFormat;
        HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &featureData, sizeof(featureData));
        if (FAILED(hr))
            return result;

        if (featureData.Support1 & (D3D12_FORMAT_SUPPORT1_TEXTURE1D | D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_TEXTURE3D | D3D12_FORMAT_SUPPORT1_TEXTURECUBE))
            result |= RHIPixelFormatSupport::Texture;
        if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
            result |= RHIPixelFormatSupport::DepthStencil;
        if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
            result |= RHIPixelFormatSupport::RenderTarget;
        if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
            result |= RHIPixelFormatSupport::Blendable;

        if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
            result |= RHIPixelFormatSupport::ShaderLoad;
        if (featureData.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
            result |= RHIPixelFormatSupport::ShaderSample;
        if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_ATOMIC_ADD)
            result |= RHIPixelFormatSupport::ShaderAtomic;
        if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD)
            result |= RHIPixelFormatSupport::ShaderUavLoad;
        if (featureData.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE)
            result |= RHIPixelFormatSupport::ShaderUavStore;

        RHITextureSampleCount supportedSampleCount = RHITextureSampleCount::Count1;
        for (uint32_t sampleCount = 1; sampleCount <= D3D12_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount *= 2)
        {
            D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS featureDataQualityLevels = { dxgiFormat, sampleCount, {}, {} };

            hr = device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &featureDataQualityLevels, sizeof(featureDataQualityLevels));
            if (SUCCEEDED(hr) && featureDataQualityLevels.NumQualityLevels > 0)
                supportedSampleCount |= static_cast<RHITextureSampleCount>(sampleCount);
        }

        return result;
    }

    bool D3D12Device::QueryVertexFormatSupport(RHIVertexAttributeFormat format)
    {
        const DXGI_FORMAT dxgiFormat = (DXGI_FORMAT)ToDxgiFormat(format);
        if (dxgiFormat == DXGI_FORMAT_UNKNOWN)
            return false;

        D3D12_FEATURE_DATA_FORMAT_SUPPORT featureData = {};
        featureData.Format = dxgiFormat;
        const HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &featureData, sizeof(featureData));
        if (FAILED(hr))
            return false;

        return true;
    }

    RHINativeHandle D3D12Device::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            //case RHINativeHandleType::DXGI_Factory:
            //    return RHINativeHandle(dxgiFactory4.Get());
            case RHINativeHandleType::DXGI_Adapter:
                return RHINativeHandle(_adapter->GetHandle());
            case RHINativeHandleType::D3D12_Device:
                return RHINativeHandle(device);
            default:
                return nullptr;
        }
    }

    RHIAdapter* D3D12Device::GetAdapter() const
    {
        return _adapter;
    }

    void D3D12Device::CopyAllocator::Init(D3D12Device* device_)
    {
        device = device_;

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH; // High priority copy queue
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        ThrowIfFailed(device->device->CreateCommandQueue(&queueDesc, PPV_ARGS(queue)));
        ThrowIfFailed(queue->SetName(L"CopyAllocator"));
    }

    void D3D12Device::CopyAllocator::Shutdown()
    {
        for (auto& context : freeList)
        {
            SafeRelease(context.commandAllocator);
            SafeRelease(context.commandList);
            SafeRelease(context.fence);

            context.uploadBuffer.Reset();
            context.uploadBufferData = nullptr;
        }

        SafeRelease(queue);
    }

    D3D12UploadContext D3D12Device::CopyAllocator::Allocate(uint64_t size)
    {
        D3D12UploadContext context;

        locker.lock();

        // Try to search for a staging buffer that can fit the request:
        for (size_t i = 0; i < freeList.size(); ++i)
        {
            if (freeList[i].uploadBuffer != nullptr &&
                freeList[i].uploadBuffer->GetSize() >= size)
            {
                if (freeList[i].IsCompleted())
                {
                    ThrowIfFailed(freeList[i].fence->Signal(0));
                    context = std::move(freeList[i]);
                    std::swap(freeList[i], freeList.back());
                    freeList.pop_back();
                    break;
                }
            }
        }
        locker.unlock();

        // If no buffer was found that fits the data, create one:
        if (!context.IsValid())
        {
            ThrowIfFailed(
                device->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&context.commandAllocator))
            );
            ThrowIfFailed(
                device->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, context.commandAllocator, nullptr, IID_PPV_ARGS(&context.commandList))
            );

            ThrowIfFailed(context.commandList->Close());

            ThrowIfFailed(device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context.fence)));

            RHIBufferDesc uploadBufferDesc;
            uploadBufferDesc.label = "CopyAllocator::UploadBuffer";
            uploadBufferDesc.size = NextPowerOfTwo(size);
            uploadBufferDesc.memoryType = RHIMemoryType::Upload;

            context.uploadBuffer = StaticCast<D3D12Buffer>(device->CreateBuffer(uploadBufferDesc, nullptr));
            ALIMER_ASSERT(context.uploadBuffer != nullptr);
            context.uploadBufferData = context.uploadBuffer->pMappedData;
        }

        // begin command list in valid state:
        ThrowIfFailed(context.commandAllocator->Reset());
        ThrowIfFailed(context.commandList->Reset(context.commandAllocator, nullptr));

        return context;
    }

    void D3D12Device::CopyAllocator::Submit(D3D12UploadContext context)
    {
        locker.lock();
        context.fenceValueSignaled++;
        freeList.push_back(context);
        locker.unlock();

        ThrowIfFailed(context.commandList->Close());
        ID3D12CommandList* commandLists[] = {
            context.commandList
        };

        queue->ExecuteCommandLists(1, commandLists);
        ThrowIfFailed(queue->Signal(context.fence, context.fenceValueSignaled));

        ThrowIfFailed(device->GetGraphicsQueue().handle->Wait(context.fence, context.fenceValueSignaled));
        ThrowIfFailed(device->GetComputeQueue().handle->Wait(context.fence, context.fenceValueSignaled));
        ThrowIfFailed(device->GetCopyQueue().handle->Wait(context.fence, context.fenceValueSignaled));
        if (device->GetVideoDecode().handle)
        {
            ThrowIfFailed(device->GetVideoDecode().handle->Wait(context.fence, context.fenceValueSignaled));
        }
    }

    /* D3D12Adapter */
    RHIDeviceRef D3D12Adapter::CreateDevice(const RHIDeviceDesc& desc)
    {
        SharedPtr<D3D12Device> device(new D3D12Device(this, desc));
        if (!device->GetHandle())
        {
            return nullptr;
        }

        return device;
    }

    RHINativeHandle D3D12Adapter::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            //case RHINativeHandleType::DXGI_Factory:
            //    return RHINativeHandle(dxgiFactory4.Get());
            case RHINativeHandleType::DXGI_Adapter:
                return RHINativeHandle(handle.Get());
            default:
                return nullptr;
        }
    }

    /* D3D12Factory */
    bool D3D12Factory::IsSupported()
    {
        static bool available_initialized = false;
        static bool available = false;

        if (available_initialized)
            return available;

        available_initialized = true;

        // Linux:  const char* libName = "libdxvk_dxgi.so";

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        lib_dxgi = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        lib_d3d12 = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (lib_dxgi == nullptr || lib_d3d12 == nullptr)
        {
            return false;
        }

        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(lib_dxgi, "CreateDXGIFactory2");
        if (CreateDXGIFactory2 == nullptr)
        {
            return false;
        }

#if defined(_DEBUG)
        DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)GetProcAddress(lib_dxgi, "DXGIGetDebugInterface1");
#endif

        // Use new D3D12GetInterface and agility SDK
        static PFN_D3D12_GET_INTERFACE func_D3D12GetInterface = (PFN_D3D12_GET_INTERFACE)GetProcAddress(lib_d3d12, "D3D12GetInterface");
        if (func_D3D12GetInterface)
        {
            ComPtr<ID3D12SDKConfiguration> sdkConfig;
            if (SUCCEEDED(func_D3D12GetInterface(CLSID_D3D12SDKConfiguration_Alimer, IID_PPV_ARGS(sdkConfig.GetAddressOf()))))
            {
                ComPtr<ID3D12SDKConfiguration1> sdkConfig1 = nullptr;
                if (SUCCEEDED(sdkConfig.As(&sdkConfig1)))
                {
                    uint32_t agilitySdkVersion = D3D12_SDK_VERSION;
                    std::string agilitySdkPath = ".\\D3D12\\"; // D3D12SDKPath;
                    if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, agilitySdkPath.c_str(), IID_PPV_ARGS(deviceFactory.GetAddressOf()))))
                    {
                        func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(deviceFactory.GetAddressOf()));
                    }
                    else if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, ".\\", IID_PPV_ARGS(deviceFactory.GetAddressOf()))))
                    {
                        func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(deviceFactory.GetAddressOf()));
                    }
                }
            }
        }

        if (!deviceFactory)
        {
            D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(lib_d3d12, "D3D12CreateDevice");
            if (!D3D12CreateDevice)
            {
                return false;
            }

            D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(lib_d3d12, "D3D12GetDebugInterface");
            D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(lib_d3d12, "D3D12SerializeVersionedRootSignature");
            if (!D3D12SerializeVersionedRootSignature) {
                return false;
            }
        }

#endif
        ComPtr<IDXGIFactory4> dxgiFactory;
        if (FAILED(CreateDXGIFactory2(0, PPV_ARGS(dxgiFactory))))
        {
            return false;
        }

        // Try to load PIX (WinPixEventRuntime.dll)
        HMODULE WinPixEventRuntimeDLL = LoadLibraryW(L"WinPixEventRuntime.dll");
        if (WinPixEventRuntimeDLL != nullptr)
        {
            PIXBeginEventOnCommandList = (PFN_PIXBeginEventOnCommandList)GetProcAddress(WinPixEventRuntimeDLL, "PIXBeginEventOnCommandList");
            PIXEndEventOnCommandList = (PFN_PIXEndEventOnCommandList)GetProcAddress(WinPixEventRuntimeDLL, "PIXEndEventOnCommandList");
            PIXSetMarkerOnCommandList = (PFN_PIXSetMarkerOnCommandList)GetProcAddress(WinPixEventRuntimeDLL, "PIXSetMarkerOnCommandList");
        }

        ComPtr<IDXGIAdapter1> dxgiAdapter;
        bool foundCompatibleDevice = false;
        for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, dxgiAdapter.ReleaseAndGetAddressOf()); ++i)
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(dxgiAdapter->GetDesc1(&adapterDesc));

            // Don't select the Basic Render Driver adapter.
            if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
            if (deviceFactory != nullptr)
            {
                if (SUCCEEDED(deviceFactory->CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
                    foundCompatibleDevice = true;
                    break;
                }
            }
            else
            {
                if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
                    foundCompatibleDevice = true;
                    break;
                }
            }
        }

        if (foundCompatibleDevice)
        {
            available = true;
            return true;
        }

        return false;
    }

    D3D12Factory::D3D12Factory(const RHIFactoryDesc& desc)
        : validationMode(desc.validationMode)
    {
        UINT dxgiFactoryFlags = 0u;
        if (validationMode != RHIValidationMode::Disabled)
        {
            dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

            ComPtr<ID3D12Debug> debugController;
            HRESULT hr = E_FAIL;
            if (deviceFactory)
            {
                hr = deviceFactory->GetConfigurationInterface(CLSID_D3D12Debug_Alimer, IID_PPV_ARGS(debugController.GetAddressOf()));
            }
            else
            {
                hr = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
            }

            if (SUCCEEDED(hr))
            {
                debugController->EnableDebugLayer();

                if (validationMode == RHIValidationMode::GPU)
                {
                    ComPtr<ID3D12Debug1> debugController1;
                    if (SUCCEEDED(debugController.As(&debugController1)))
                    {
                        debugController1->SetEnableGPUBasedValidation(TRUE);
                        debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                    }

                    ComPtr<ID3D12Debug2> debugController2;
                    if (SUCCEEDED(debugController.As(&debugController2)))
                    {
                        const bool g_D3D12DebugLayer_GPUBasedValidation_StateTracking_Enabled = true;

                        if (g_D3D12DebugLayer_GPUBasedValidation_StateTracking_Enabled)
                            debugController2->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_DISABLE_STATE_TRACKING);
                        else
                            debugController2->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                    }
                }

                // DRED
                ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> pDredSettings;
                if (deviceFactory)
                {
                    hr = deviceFactory->GetConfigurationInterface(CLSID_D3D12DeviceRemovedExtendedData_Alimer, IID_PPV_ARGS(pDredSettings.GetAddressOf()));
                }
                else
                {
                    hr = D3D12GetDebugInterface(IID_PPV_ARGS(pDredSettings.GetAddressOf()));
                }

                if (SUCCEEDED(hr))
                {
                    // Turn on auto-breadcrumbs and page fault reporting.
                    pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    pDredSettings->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }

#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
                ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
                if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
                {
                    dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

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
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }
        }

        // Create factory and determines whether tearing support is available for fullscreen borderless windows.
        {
            ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(handle.ReleaseAndGetAddressOf())));

            BOOL allowTearing = FALSE;
            ComPtr<IDXGIFactory5> dxgiFactory5;
            HRESULT hr = handle.As(&dxgiFactory5);
            if (SUCCEEDED(hr))
            {
                hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            }

            if (FAILED(hr) || !allowTearing)
            {
                tearingSupported = false;
            }
            else
            {
                tearingSupported = true;
            }
        }
    }

    D3D12Factory::~D3D12Factory()
    {

    }

    RHISurfaceRef D3D12Factory::CreateSurface(void* window, void* display)
    {
        return nullptr;
    }

    RHINativeHandle D3D12Factory::GetNativeHandle(RHINativeHandleType objectType)
    {
        switch (objectType)
        {
            case RHINativeHandleType::DXGI_Factory:
                return RHINativeHandle(handle.Get());
            default:
                return nullptr;
        }
    }

    bool D3D12_IsSupported()
    {
        return D3D12Factory::IsSupported();
    }

    RHIFactoryRef D3D12_CreateFactory(const RHIFactoryDesc& desc)
    {
        if (!D3D12Factory::IsSupported())
        {
            LOGE("D3D12 is not supported on this system.");
            return nullptr;
        }

        SharedPtr<D3D12Factory> factory(new D3D12Factory(desc));
        return factory;
    }
}

#endif /* defined(ALIMER_RHI_D3D12) */
