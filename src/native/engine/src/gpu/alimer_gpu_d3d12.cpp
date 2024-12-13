// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_GPU_D3D12)
#include "alimer_gpu_internal.h"

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
//#   include <directx/d3dx12_resource_helpers.h>
//#   include <directx/d3dx12_pipeline_state_stream.h>
#   include <directx/d3dx12_check_feature_support.h>
//#   include <dcomp.h>
#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//#   include "ThirdParty/microsoft.ui.xaml.media.dxinterop.h"
#else
#   include <windows.ui.xaml.media.dxinterop.h> // WinRT
#endif
#   define PPV_ARGS(x) IID_PPV_ARGS(&x)
#endif

#define VHR(hr) do \
{ \
  if (FAILED(hr)) { \
    alimerLogError(LogCategory_GPU, "[%s()] HRESULT error detected (0x%lX)", __FUNCTION__, hr); \
    assert(false); \
    ExitProcess(1); \
  } \
} while(0)

#define D3D12_LOG_ERROR(hr, message) alimerLogError(LogCategory_GPU, "%s, result: 0x%08X", message, hr);

#include <wrl/client.h>

#include <dxgi1_6.h>

#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <dxgidebug.h>
// Declare debug guids to avoid linking with "dxguid.lib"
static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };
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

#include <inttypes.h>
#include <sstream>
#include <mutex>
#include <vector>
#include <deque>
#include <unordered_map>

using Microsoft::WRL::ComPtr;

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace
{
    const char* ToString(D3D12_MESSAGE_CATEGORY category)
    {
        switch (category)
        {
            case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:
                return "APPLICATION_DEFINED";

            case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:
                return "MISCELLANEOUS";
            case D3D12_MESSAGE_CATEGORY_INITIALIZATION:
                return "INITIALIZATION";
            case D3D12_MESSAGE_CATEGORY_CLEANUP:
                return "CLEANUP";
            case D3D12_MESSAGE_CATEGORY_COMPILATION:
                return "COMPILATION";
            case D3D12_MESSAGE_CATEGORY_STATE_CREATION:
                return "STATE_CREATION";
            case D3D12_MESSAGE_CATEGORY_STATE_SETTING:
                return "STATE_SETTING";
            case D3D12_MESSAGE_CATEGORY_STATE_GETTING:
                return "STATE_GETTING";
            case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
                return "RESOURCE_MANIPULATION";
            case D3D12_MESSAGE_CATEGORY_EXECUTION:
                return "EXECUTION";
            case D3D12_MESSAGE_CATEGORY_SHADER:
                return "SHADER";
            default:
                return "UNKNOWN";
        }
    }

    const char* ToString(D3D12_MESSAGE_SEVERITY category)
    {
        switch (category)
        {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                return "CORRUPTION";
            case D3D12_MESSAGE_SEVERITY_ERROR:
                return "ERROR";
            case D3D12_MESSAGE_SEVERITY_WARNING:
                return "WARNING";
            case D3D12_MESSAGE_SEVERITY_INFO:
                return "INFO";
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                return "MESSAGE";
            default:
                return "UNKNOWN";
        }
    }

    inline void __stdcall DebugMessageCallback(
        D3D12_MESSAGE_CATEGORY Category,
        D3D12_MESSAGE_SEVERITY Severity,
        D3D12_MESSAGE_ID ID,
        LPCSTR pDescription,
        [[maybe_unused]] void* pContext)
    {
        const char* categoryStr = ToString(Category);
        const char* severityStr = ToString(Severity);
        if (Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION || Severity == D3D12_MESSAGE_SEVERITY_ERROR)
        {
            alimerLogError(LogCategory_GPU, "D3D12 %s: %s [%s #%d]", severityStr, pDescription, categoryStr, ID);
        }
        else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
        {
            alimerLogWarn(LogCategory_GPU, "D3D12 %s: %s [%s #%d]", severityStr, pDescription, categoryStr, ID);
        }
        else
        {
            alimerLogInfo(LogCategory_GPU, "D3D12 %s: %s [%s #%d]", severityStr, pDescription, categoryStr, ID);
        }
    }

    constexpr D3D12_COMMAND_LIST_TYPE ToD3D12(GPUQueueType type)
    {
        switch (type)
        {
            case GPUQueueType_Graphics:
                return D3D12_COMMAND_LIST_TYPE_DIRECT;

            case GPUQueueType_Compute:
                return D3D12_COMMAND_LIST_TYPE_COMPUTE;

            case GPUQueueType_Copy:
                return D3D12_COMMAND_LIST_TYPE_COPY;

            case GPUQueueType_VideoDecode:
                return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;

            default:
                ALIMER_UNREACHABLE();
        }
    }

    inline DXGI_FORMAT ToDxgiRTVFormat(PixelFormat format)
    {
        return static_cast<DXGI_FORMAT>(alimerPixelFormatToDxgiFormat(format));
    }

    [[maybe_unused]] inline DXGI_FORMAT ToDxgiDSVFormat(PixelFormat format)
    {
        return static_cast<DXGI_FORMAT>(alimerPixelFormatToDxgiFormat(format));
    }

    [[maybe_unused]] inline DXGI_FORMAT ToDxgiSRVFormat(PixelFormat format)
    {
        // Try to resolve resource format:
        switch (format)
        {
            case PixelFormat_Depth16Unorm:
                return DXGI_FORMAT_R16_UNORM;

            case PixelFormat_Depth32Float:
                return DXGI_FORMAT_R32_FLOAT;

                //case PixelFormat_Stencil8:
            case PixelFormat_Depth24UnormStencil8:
                return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

            case PixelFormat_Depth32FloatStencil8:
                return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

                //case PixelFormat::NV12:
                //    srvDesc.Format = DXGI_FORMAT_R8_UNORM;
                //    break;
            default:
                return static_cast<DXGI_FORMAT>(alimerPixelFormatToDxgiFormat(format));
        }
    }

    [[maybe_unused]] inline DXGI_FORMAT ToDxgiUAVFormat(PixelFormat format)
    {
        return static_cast<DXGI_FORMAT>(alimerPixelFormatToDxgiFormat(format));
    }

    [[maybe_unused]] constexpr DXGI_FORMAT ToDxgiFormat(GPUVertexFormat format)
    {
        switch (format)
        {
            case GPUVertexFormat_UByte:               return DXGI_FORMAT_R8_UINT;
            case GPUVertexFormat_UByte2:              return DXGI_FORMAT_R8G8_UINT;
            case GPUVertexFormat_UByte4:              return DXGI_FORMAT_R8G8B8A8_UINT;
            case GPUVertexFormat_Byte:                return DXGI_FORMAT_R8_SINT;
            case GPUVertexFormat_Byte2:               return DXGI_FORMAT_R8G8_SINT;
            case GPUVertexFormat_Byte4:               return DXGI_FORMAT_R8G8B8A8_SINT;
            case GPUVertexFormat_UByteNormalized:     return DXGI_FORMAT_R8_UNORM;
            case GPUVertexFormat_UByte2Normalized:    return DXGI_FORMAT_R8G8_UNORM;
            case GPUVertexFormat_UByte4Normalized:    return DXGI_FORMAT_R8G8B8A8_UNORM;
            case GPUVertexFormat_ByteNormalized:      return DXGI_FORMAT_R8_SNORM;
            case GPUVertexFormat_Byte2Normalized:     return DXGI_FORMAT_R8G8_SNORM;
            case GPUVertexFormat_Byte4Normalized:     return DXGI_FORMAT_R8G8B8A8_SNORM;

            case GPUVertexFormat_UShort:              return DXGI_FORMAT_R16_UINT;
            case GPUVertexFormat_UShort2:             return DXGI_FORMAT_R16G16_UINT;
            case GPUVertexFormat_UShort4:             return DXGI_FORMAT_R16G16B16A16_UINT;
            case GPUVertexFormat_Short:               return DXGI_FORMAT_R16_SINT;
            case GPUVertexFormat_Short2:              return DXGI_FORMAT_R16G16_SINT;
            case GPUVertexFormat_Short4:              return DXGI_FORMAT_R16G16B16A16_SINT;
            case GPUVertexFormat_UShortNormalized:    return DXGI_FORMAT_R16_UNORM;
            case GPUVertexFormat_UShort2Normalized:   return DXGI_FORMAT_R16G16_UNORM;
            case GPUVertexFormat_UShort4Normalized:   return DXGI_FORMAT_R16G16B16A16_UNORM;
            case GPUVertexFormat_ShortNormalized:     return DXGI_FORMAT_R16_SNORM;
            case GPUVertexFormat_Short2Normalized:    return DXGI_FORMAT_R16G16_SNORM;
            case GPUVertexFormat_Short4Normalized:    return DXGI_FORMAT_R16G16B16A16_SNORM;

            case GPUVertexFormat_Half:                return DXGI_FORMAT_R16_FLOAT;
            case GPUVertexFormat_Half2:               return DXGI_FORMAT_R16G16_FLOAT;
            case GPUVertexFormat_Half4:               return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case GPUVertexFormat_Float:               return DXGI_FORMAT_R32_FLOAT;
            case GPUVertexFormat_Float2:              return DXGI_FORMAT_R32G32_FLOAT;
            case GPUVertexFormat_Float3:              return DXGI_FORMAT_R32G32B32_FLOAT;
            case GPUVertexFormat_Float4:              return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case GPUVertexFormat_UInt:                return DXGI_FORMAT_R32_UINT;
            case GPUVertexFormat_UInt2:               return DXGI_FORMAT_R32G32_UINT;
            case GPUVertexFormat_UInt3:               return DXGI_FORMAT_R32G32B32_UINT;
            case GPUVertexFormat_UInt4:               return DXGI_FORMAT_R32G32B32A32_UINT;

            case GPUVertexFormat_Int:                 return DXGI_FORMAT_R32_SINT;
            case GPUVertexFormat_Int2:                return DXGI_FORMAT_R32G32_SINT;
            case GPUVertexFormat_Int3:                return DXGI_FORMAT_R32G32B32_SINT;
            case GPUVertexFormat_Int4:                return DXGI_FORMAT_R32G32B32A32_SINT;

            case GPUVertexFormat_Unorm10_10_10_2:     return DXGI_FORMAT_R10G10B10A2_UNORM;
            case GPUVertexFormat_Unorm8x4BGRA:        return DXGI_FORMAT_B8G8R8A8_UNORM;

            default:
                ALIMER_UNREACHABLE();
        }
    }

    constexpr DXGI_FORMAT GetTypelessFormatFromDepthFormat(PixelFormat format)
    {
        bool UsePackedDepth24UnormStencil8Format = true;

        switch (format)
        {
            //case PixelFormat_Stencil8:
            //    return DXGI_FORMAT_R24G8_TYPELESS;
            case PixelFormat_Depth16Unorm:
                return DXGI_FORMAT_R16_TYPELESS;

            case PixelFormat_Depth32Float:
                return DXGI_FORMAT_R32_TYPELESS;

            case PixelFormat_Depth24UnormStencil8:
                return UsePackedDepth24UnormStencil8Format ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R32G8X24_TYPELESS;
            case PixelFormat_Depth32FloatStencil8:
                return DXGI_FORMAT_R32G8X24_TYPELESS;

            default:
                ALIMER_ASSERT(alimerPixelFormatIsDepth(format) == false);
                return (DXGI_FORMAT)alimerPixelFormatToDxgiFormat(format);
        }
    }

    constexpr DXGI_FORMAT ToDxgiSwapChainFormat(PixelFormat format)
    {
        // FLIP_DISCARD and FLIP_SEQEUNTIAL swapchain buffers only support these formats
        switch (format)
        {
            case PixelFormat_RGBA16Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case PixelFormat_BGRA8Unorm:
            case PixelFormat_BGRA8UnormSrgb:
                return DXGI_FORMAT_B8G8R8A8_UNORM;

            case PixelFormat_RGBA8Unorm:
            case PixelFormat_RGBA8UnormSrgb:
                return DXGI_FORMAT_R8G8B8A8_UNORM;

            case PixelFormat_RGB10A2Unorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;

            default:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
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

    [[maybe_unused]] constexpr uint32_t GetPlaneSlice(DXGI_FORMAT format, GPUTextureAspect aspect)
    {
        switch (aspect)
        {
            case GPUTextureAspect_All:
            case GPUTextureAspect_DepthOnly:
                return 0;
            case GPUTextureAspect_StencilOnly:
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

    struct D3D12TextureLayoutMapping final
    {
        D3D12_BARRIER_LAYOUT layout;
        D3D12_BARRIER_SYNC sync;
        D3D12_BARRIER_ACCESS access;

        D3D12TextureLayoutMapping(D3D12_BARRIER_LAYOUT layout_, D3D12_BARRIER_SYNC sync_, D3D12_BARRIER_ACCESS access_)
            : layout(layout_)
            , sync(sync_)
            , access(access_)
        {
        }
    };

    D3D12TextureLayoutMapping ConvertTextureLayout(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout::Undefined:
                return {
                    D3D12_BARRIER_LAYOUT_COMMON,
                    D3D12_BARRIER_SYNC_NONE,
                    D3D12_BARRIER_ACCESS_COMMON
                };

            case TextureLayout::CopySource:
                return {
                    D3D12_BARRIER_LAYOUT_COPY_SOURCE,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_SOURCE
                };

            case TextureLayout::CopyDest:
                return {
                    D3D12_BARRIER_LAYOUT_COPY_DEST,
                    D3D12_BARRIER_SYNC_COPY,
                    D3D12_BARRIER_ACCESS_COPY_DEST
                };

            case TextureLayout::ResolveSource:
                return {
                    D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_SOURCE
                };

            case TextureLayout::ResolveDest:
                return {
                    D3D12_BARRIER_LAYOUT_RESOLVE_DEST,
                    D3D12_BARRIER_SYNC_RESOLVE,
                    D3D12_BARRIER_ACCESS_RESOLVE_DEST
                };

            case TextureLayout::ShaderResource:
                return {
                    D3D12_BARRIER_LAYOUT_SHADER_RESOURCE,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_SHADER_RESOURCE
                };

            case TextureLayout::UnorderedAccess:
                return {
                    D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS,
                    D3D12_BARRIER_SYNC_ALL_SHADING,
                    D3D12_BARRIER_ACCESS_UNORDERED_ACCESS
                };

            case TextureLayout::RenderTarget:
                return {
                    D3D12_BARRIER_LAYOUT_RENDER_TARGET,
                    D3D12_BARRIER_SYNC_RENDER_TARGET,
                    D3D12_BARRIER_ACCESS_RENDER_TARGET };

            case TextureLayout::DepthWrite:
                return {
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE,
                    D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                    D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE
                };

            case TextureLayout::DepthRead:
                return {
                    D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ,
                    D3D12_BARRIER_SYNC_DEPTH_STENCIL,
                    D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ
                };

            case TextureLayout::Present:
                return {
                    D3D12_BARRIER_LAYOUT_PRESENT,
                    D3D12_BARRIER_SYNC_ALL,
                    D3D12_BARRIER_ACCESS_COMMON
                };

            case TextureLayout::ShadingRateSurface:
                return {
                    D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE,
                    D3D12_BARRIER_SYNC_PIXEL_SHADING,
                    D3D12_BARRIER_ACCESS_SHADING_RATE_SOURCE
                };


            default:
                ALIMER_UNREACHABLE();
        }
    }

    D3D12_RESOURCE_STATES ConvertTextureLayoutLegacy(TextureLayout layout)
    {
        switch (layout)
        {
            case TextureLayout::Undefined:
                return D3D12_RESOURCE_STATE_COMMON;

            case TextureLayout::CopySource:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;

            case TextureLayout::CopyDest:
                return D3D12_RESOURCE_STATE_COPY_DEST;

            case TextureLayout::ShaderResource:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            case TextureLayout::UnorderedAccess:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

            case TextureLayout::RenderTarget:
                return D3D12_RESOURCE_STATE_RENDER_TARGET;

            case TextureLayout::DepthWrite:
                return D3D12_RESOURCE_STATE_DEPTH_WRITE;

            case TextureLayout::DepthRead:
                return D3D12_RESOURCE_STATE_DEPTH_READ;

            case TextureLayout::Present:
                return D3D12_RESOURCE_STATE_PRESENT;

            case TextureLayout::ShadingRateSurface:
                return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;

            default:
                ALIMER_UNREACHABLE();
        }
    }
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);

const GUID CLSID_D3D12SDKConfiguration_Alimer = { 0x7cda6aca, 0xa03e, 0x49c8, {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce} };
const GUID CLSID_D3D12DeviceFactory_Alimer = { 0x114863bf, 0xc386, 0x4aee, {0xb3, 0x9d, 0x8f, 0x0b, 0xbb, 0x06, 0x29, 0x55} };
const GUID CLSID_D3D12Debug_Alimer = { 0xf2352aeb, 0xdd84, 0x49fe, {0xb9, 0x7b, 0xa9, 0xdc, 0xfd, 0xcc, 0x1b, 0x4f} };
const GUID CLSID_D3D12DeviceRemovedExtendedData_Alimer = { 0x4a75bbc4, 0x9ff4, 0x4ad8, {0x9f, 0x18, 0xab, 0xae, 0x84, 0xdc, 0x5f, 0xf2} };

/* pix3 */
void WINAPI PIXBeginEventOnCommandListFn(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
void WINAPI PIXEndEventOnCommandListFn(ID3D12GraphicsCommandList* commandList);
void WINAPI PIXSetMarkerOnCommandListFn(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);
//void WINAPI PIXBeginEventOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);
//void WINAPI PIXEndEventOnCommandQueue(ID3D12CommandQueue* commandQueue);
//void WINAPI PIXSetMarkerOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);
using PFN_PIXBeginEventOnCommandList = decltype(&PIXBeginEventOnCommandListFn);
using PFN_PIXEndEventOnCommandList = decltype(&PIXEndEventOnCommandListFn);
using PFN_PIXSetMarkerOnCommandList = decltype(&PIXSetMarkerOnCommandListFn);

struct D3D12_State {
    HMODULE lib_dxgi = nullptr;
    HMODULE lib_d3d12 = nullptr;
    HMODULE lib_WinPixEventRuntime = nullptr;

    PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

#if defined(_DEBUG)
    PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;
#endif

    PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

    PFN_PIXBeginEventOnCommandList PIXBeginEventOnCommandList = nullptr;
    PFN_PIXEndEventOnCommandList PIXEndEventOnCommandList = nullptr;
    PFN_PIXSetMarkerOnCommandList PIXSetMarkerOnCommandList = nullptr;

    ComPtr<ID3D12DeviceFactory> deviceFactory;

    ~D3D12_State()
    {
        deviceFactory.Reset();

        if (lib_d3d12)
        {
            FreeLibrary(lib_d3d12);
            lib_d3d12 = nullptr;
        }

        if (lib_dxgi)
        {
            FreeLibrary(lib_dxgi);
            lib_dxgi = nullptr;
        }

        if (lib_WinPixEventRuntime)
        {
            FreeLibrary(lib_WinPixEventRuntime);
            lib_WinPixEventRuntime = nullptr;
        }
    }
} d3d12_state;

#define dxgi_CreateDXGIFactory2 d3d12_state.CreateDXGIFactory2

#if defined(_DEBUG)
#define dxgi_DXGIGetDebugInterface1 d3d12_state.DXGIGetDebugInterface1
#endif /* defined(_DEBUG) */

#define d3d12_D3D12CreateDevice d3d12_state.D3D12CreateDevice
#define d3d12_D3D12GetDebugInterface d3d12_state.D3D12GetDebugInterface
#define d3d12_D3D12SerializeVersionedRootSignature d3d12_state.D3D12SerializeVersionedRootSignature
#else
#define dxgi_CreateDXGIFactory2 CreateDXGIFactory2
#define d3d12_D3D12CreateDevice D3D12CreateDevice
#define d3d12_D3D12GetDebugInterface D3D12GetDebugInterface
#define d3d12_D3D12SerializeVersionedRootSignature D3D12SerializeVersionedRootSignature
#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */

static_assert(sizeof(GPUViewport) == sizeof(D3D12_VIEWPORT), "Viewport mismatch");
static_assert(offsetof(GPUViewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "Layout mismatch");
static_assert(offsetof(GPUViewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "Layout mismatch");
static_assert(offsetof(GPUViewport, width) == offsetof(D3D12_VIEWPORT, Width), "Layout mismatch");
static_assert(offsetof(GPUViewport, height) == offsetof(D3D12_VIEWPORT, Height), "Layout mismatch");
static_assert(offsetof(GPUViewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "Layout mismatch");
static_assert(offsetof(GPUViewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "Layout mismatch");

static_assert(sizeof(GPUDispatchIndirectCommand) == sizeof(D3D12_DISPATCH_ARGUMENTS), "DispatchIndirectCommand mismatch");
static_assert(offsetof(GPUDispatchIndirectCommand, groupCountX) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountX), "Layout mismatch");
static_assert(offsetof(GPUDispatchIndirectCommand, groupCountY) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountY), "Layout mismatch");
static_assert(offsetof(GPUDispatchIndirectCommand, groupCountZ) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountZ), "Layout mismatch");

struct D3D12CommandBuffer;
struct D3D12Queue;
struct D3D12Device;
struct D3D12Surface;
struct D3D12Adapter;
struct D3D12Instance;

using DescriptorIndex = uint32_t;
using RootParameterIndex = uint32_t;

static constexpr DescriptorIndex kInvalidDescriptorIndex = ~0u;

struct D3D12Resource
{
    D3D12Device* device = nullptr;
    ID3D12Resource* handle = nullptr;
    D3D12MA::Allocation* allocation = nullptr;
    bool immutableState = false;
};

struct D3D12Buffer final : public GPUBufferImpl, public D3D12Resource
{
    uint64_t allocatedSize = 0;
    D3D12_GPU_VIRTUAL_ADDRESS deviceAddress = 0;
    void* pMappedData = nullptr;
    HANDLE sharedHandle = nullptr;
    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_COMMON;

    ~D3D12Buffer() override;
    void SetLabel(const char* label) override;
    GPUDeviceAddress GetDeviceAddress() const override { return deviceAddress; }
};

struct D3D12Texture final : public GPUTextureImpl, public D3D12Resource
{
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
    HANDLE sharedHandle = nullptr;
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footPrints;
    std::vector<uint64_t> rowSizesInBytes;
    std::vector<uint32_t> numRows;
    uint64_t allocatedSize = 0;
    uint32_t numSubResources = 0;
    mutable std::vector<TextureLayout> subResourcesStates;
    mutable std::unordered_map<size_t, DescriptorIndex> RTVs;
    mutable std::unordered_map<size_t, DescriptorIndex> DSVs;

    ~D3D12Texture() override;
    void SetLabel(const char* label) override;
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(DXGI_FORMAT rtvFormat, uint32_t mipLevel) const;
};

struct D3D12ComputePassEncoder final : public GPUComputePassEncoderImpl
{
    D3D12CommandBuffer* commandBuffer = nullptr;
    bool hasLabel = false;

    void Begin(const GPUComputePassDesc& desc);
    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void PrepareDispatch();
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
    void DispatchIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset) override;
};

struct D3D12RenderPassEncoder final : public GPURenderPassEncoderImpl
{
    D3D12CommandBuffer* commandBuffer = nullptr;
    D3D12_RENDER_PASS_RENDER_TARGET_DESC RTVs[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC DSV = {};
    D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE;
    bool hasLabel = false;

    void Begin(const GPURenderPassDesc& desc);
    void EndEncoding() override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    void SetViewport(const GPUViewport* viewport) override;
    void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) override;
    void SetScissorRect(const GPUScissorRect* scissorRect) override;
    void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) override;
    void SetStencilReference(uint32_t reference) override;
};

struct D3D12CommandBuffer final : public GPUCommandBufferImpl
{
    static constexpr uint32_t kMaxBarrierCount = 16;

    D3D12Queue* queue = nullptr;
    uint32_t index = 0;
    bool hasLabel = false;
    bool encoderActive = false;
    D3D12ComputePassEncoder* computePassEncoder = nullptr;
    D3D12RenderPassEncoder* renderPassEncoder = nullptr;

    ID3D12CommandAllocator* commandAllocators[GPU_MAX_INFLIGHT_FRAMES] = {};
    ID3D12GraphicsCommandList6* commandList = nullptr;
    ID3D12GraphicsCommandList7* commandList7 = nullptr;
    UINT numBarriersToCommit = 0;
    std::vector<D3D12_GLOBAL_BARRIER> globalBarriers;
    std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;
    std::vector<D3D12_BUFFER_BARRIER> bufferBarriers;
    // Legacy barriers
    D3D12_RESOURCE_BARRIER barriers[kMaxBarrierCount] = {};
    std::vector<D3D12Surface*> presentSurfaces;
    D3D12_VERTEX_BUFFER_VIEW vboViews[GPU_MAX_VERTEX_BUFFER_BINDINGS] = {};

    ~D3D12CommandBuffer() override;
    void Clear();
    void Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc);
    ID3D12CommandList* End();

    void TextureBarrier(const D3D12Texture* resource, TextureLayout newLayout, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool commit = false);
    void InsertUAVBarrier(const D3D12Resource* resource, bool commit = false);
    void CommitBarriers();

    GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface surface, GPUTexture* surfaceTexture) override;
    void PushDebugGroup(const char* groupLabel) const override;
    void PopDebugGroup() const override;
    void InsertDebugMarker(const char* markerLabel) const override;

    GPUComputePassEncoder BeginComputePass(const GPUComputePassDesc& desc) override;
    GPURenderPassEncoder BeginRenderPass(const GPURenderPassDesc& desc) override;
};

struct D3D12Queue final : public GPUQueueImpl
{
    D3D12Device* device = nullptr;
    GPUQueueType queueType = GPUQueueType_Count;
    ID3D12CommandQueue* handle = nullptr;
    ID3D12Fence* fence = nullptr;
    uint64_t nextFenceValue = 0;
    uint64_t lastCompletedFenceValue = 0;
    std::mutex fenceMutex;
    ID3D12Fence* frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};

    std::vector<D3D12CommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUQueueType GetQueueType() const override { return queueType; }
    GPUCommandBuffer AcquireCommandBuffer(const GPUCommandBufferDesc* desc) override;

    uint64_t IncrementFenceValue();
    bool IsFenceComplete(uint64_t fenceValue);
    void WaitForFenceValue(uint64_t fenceValue);
    void WaitIdle();
    void Submit(uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers) override;
};

struct D3D12UploadContext final
{
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12Fence* fence = nullptr;
    uint64_t fenceValueSignaled = 0;
    ID3D12Resource* uploadBuffer = nullptr;
    D3D12MA::Allocation* uploadBufferAllocation = nullptr;
    void* uploadBufferData = nullptr;

    inline bool IsValid() const { return commandList != nullptr; }
    inline bool IsCompleted() const { return fence->GetCompletedValue() >= fenceValueSignaled; }
};

struct D3D12CopyAllocator final
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

class D3D12DescriptorAllocator final
{
public:
    void Init(ID3D12Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE heapType_, uint32_t numDescriptors_)
    {
        device = device_;
        heapType = heapType_;
        shaderVisible = heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        stride = device_->GetDescriptorHandleIncrementSize(heapType);

        VHR(AllocateResources(numDescriptors_));
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
                alimerLogError(LogCategory_GPU, "D3D12: Failed to grow a descriptor heap!");
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
                alimerLogError(LogCategory_GPU, "D3D12: Attempted to release an un-allocated descriptor");
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
        uint32_t newSize = GetNextPowerOfTwo(minRequiredSize);

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

struct D3D12Device final : public GPUDeviceImpl
{
    D3D12Adapter* adapter = nullptr;
    ID3D12Device5* handle = nullptr;
    ComPtr<ID3D12VideoDevice> videoDevice;
    CD3DX12FeatureSupport features;
    DWORD callbackCookie = 0;
    bool shuttingDown = false;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    //ComPtr<D3D12MA::Pool> umaPool;
    ComPtr<ID3D12Fence> deviceRemovedFence;
    HANDLE deviceRemovedWaitHandle = nullptr;
#endif

    D3D12Queue queues[GPUQueueType_Count];
    ComPtr<D3D12MA::Allocator> allocator;
    D3D12CopyAllocator copyAllocator;

    D3D12DescriptorAllocator renderTargetViewHeap;
    D3D12DescriptorAllocator depthStencilViewHeap;
    D3D12DescriptorAllocator shaderResourceViewHeap;
    D3D12DescriptorAllocator samplerHeap;

    ID3D12CommandSignature* dispatchCommandSignature = nullptr;

    uint32_t maxFramesInFlight = 0;
    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;
    // Deletion queue objects
    std::mutex destroyMutex;
    std::deque<std::pair<D3D12MA::Allocation*, uint64_t>> deferredAllocations;
    std::deque<std::pair<ID3D12DeviceChild*, uint64_t>> deferredReleases;
    //std::vector<uint32_t> freeBindlessResources;
    //std::vector<uint32_t> freeBindlessSamplers;
    //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessResources;
    //std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessSamplers;

    ~D3D12Device() override;
    void OnDeviceRemoved();
    void SetLabel(const char* label) override;
    bool HasFeature(GPUFeature feature) const override;
    GPUQueue GetQueue(GPUQueueType type) override;
    bool WaitIdle() override;
    uint64_t CommitFrame() override;

    ID3D12CommandSignature* CreateCommandSignature(D3D12_INDIRECT_ARGUMENT_TYPE type, uint32_t stride);
    void DeferDestroy(ID3D12DeviceChild* resource, D3D12MA::Allocation* allocation = nullptr);
    void ProcessDeletionQueue(bool force);

    /* Resource creation */
    GPUBuffer CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) override;
    GPUTexture CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) override;
    GPUSampler CreateSampler(const GPUSamplerDesc& desc) override;
    GPUBindGroupLayout CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) override;
    GPUPipelineLayout CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) override;
    GPUShaderModule CreateShaderModule(const GPUShaderModuleDesc* desc) override;
    GPUComputePipeline CreateComputePipeline(const GPUComputePipelineDesc& desc) override;
    GPURenderPipeline CreateRenderPipeline(const GPURenderPipelineDesc& desc) override;
};

struct D3D12Surface final : public GPUSurfaceImpl
{
    D3D12Instance* instance = nullptr;
    D3D12Device* device = nullptr;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    HWND handle = nullptr;
#endif
    uint32_t width = 0;
    uint32_t height = 0;
    IDXGISwapChain3* swapChain3 = nullptr;
    uint32_t swapChainWidth = 0;
    uint32_t swapChainHeight = 0;
    uint32_t backBufferIndex = 0;
    HANDLE frameLatencyWaitableObject = INVALID_HANDLE_VALUE;
    std::vector<D3D12Texture*> backbufferTextures;

    ~D3D12Surface() override;
    GPUResult GetCapabilities(GPUAdapter adapter, GPUSurfaceCapabilities* capabilities) const override;
    bool Configure(const GPUSurfaceConfig* config_) override;
    void Unconfigure() override;
    void Present();
};

struct D3D12Adapter final : public GPUAdapterImpl
{
    D3D12Instance* instance = nullptr;
    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    char* deviceName = nullptr;
    uint16_t driverVersion[4];
    std::string driverDescription;
    GPUAdapterType adapterType = GPUAdapterType_Unknown;
    uint32_t vendorID;
    uint32_t deviceID;
    bool shaderFloat16 = false;
    bool GPUUploadHeapSupported = false;
    bool copyQueueTimestampQueriesSupported = false;
    bool cacheCoherentUMA = false;
    bool shaderOutputViewportIndex = false;
    GPUConservativeRasterizationTier conservativeRasterizationTier = GPUConservativeRasterizationTier_NotSupported;

    ~D3D12Adapter() override;
    GPUResult GetInfo(GPUAdapterInfo* info) const override;
    GPUResult GetLimits(GPULimits* limits) const override;
    bool HasFeature(GPUFeature feature) const override;
    GPUDevice CreateDevice(const GPUDeviceDesc& desc) override;
};

struct D3D12Instance final : public GPUInstance
{
    ComPtr<IDXGIFactory4> dxgiFactory4;
    bool tearingSupported = false;
    GPUValidationMode validationMode;

    ~D3D12Instance() override;
    GPUSurface CreateSurface(Window* window) override;
    GPUAdapter RequestAdapter(const GPURequestAdapterOptions* options) override;
};

/* D3D12Buffer */
D3D12Buffer::~D3D12Buffer()
{
    device->DeferDestroy(handle, allocation);
    handle = nullptr;
    allocation = nullptr;
}

void D3D12Buffer::SetLabel(const char* label)
{
    WCHAR* wideLabel = Win32_CreateWideStringFromUTF8(label);
    if (wideLabel)
    {
        handle->SetName(wideLabel);
        if (allocation)
        {
            allocation->SetName(wideLabel);
        }
        alimerFree(wideLabel);
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

void D3D12Texture::SetLabel(const char* label)
{
    WCHAR* wideLabel = Win32_CreateWideStringFromUTF8(label);
    if (wideLabel)
    {
        handle->SetName(wideLabel);
        if (allocation)
        {
            allocation->SetName(wideLabel);
        }
        alimerFree(wideLabel);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Texture::GetRTV(DXGI_FORMAT rtvFormat, uint32_t mipLevel) const
{
    size_t hash = 0;

    HashCombine(hash, (uint32_t)rtvFormat);
    HashCombine(hash, mipLevel);

    auto it = RTVs.find(hash);
    if (it == RTVs.end())
    {
        D3D12_RESOURCE_DESC resourceDesc = handle->GetDesc();

        const bool isArray = resourceDesc.DepthOrArraySize > 1;
        const bool isMultiSample = resourceDesc.SampleDesc.Count > 1;

        D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
        viewDesc.Format = rtvFormat;
        switch (desc.dimension)
        {
            case TextureDimension_1D:
                if (isArray)
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MipSlice = mipLevel;
                    viewDesc.Texture1DArray.FirstArraySlice = 0;
                    viewDesc.Texture1DArray.ArraySize = -1;
                }
                else
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MipSlice = mipLevel;
                }
                break;
            case TextureDimension_2D:
                if (isMultiSample)
                {
                    viewDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DMS;
                    if (isArray)
                    {
                        viewDesc.Texture2DMSArray.FirstArraySlice = 0;
                        viewDesc.Texture2DMSArray.ArraySize = -1;
                    }
                }
                else
                {
                    viewDesc.ViewDimension = isArray ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
                    if (isArray)
                    {
                        viewDesc.Texture2DArray.MipSlice = mipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = 0;
                        viewDesc.Texture2DArray.ArraySize = -1;
                        viewDesc.Texture2DArray.PlaneSlice = 0;
                    }
                    else
                    {
                        viewDesc.Texture2D.MipSlice = mipLevel;
                        viewDesc.Texture2D.PlaneSlice = 0;
                    }
                }
                break;
            case TextureDimension_3D:
                viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                viewDesc.Texture3D.MipSlice = mipLevel;
                viewDesc.Texture3D.FirstWSlice = 0;
                viewDesc.Texture3D.WSize = resourceDesc.DepthOrArraySize;
                break;
            case TextureDimension_Cube:
                viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                viewDesc.Texture2DArray.MipSlice = mipLevel;
                viewDesc.Texture2DArray.FirstArraySlice = 0;
                viewDesc.Texture2DArray.ArraySize = resourceDesc.DepthOrArraySize;
                viewDesc.Texture2DArray.PlaneSlice = 0;
                break;

            default:
                alimerLogError(LogCategory_GPU, "Invalid TextureView type");
                return {};
        }

        DescriptorIndex descriptorIndex = device->renderTargetViewHeap.AllocateDescriptors();
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->renderTargetViewHeap.GetCpuHandle(descriptorIndex);
        device->handle->CreateRenderTargetView(handle, &viewDesc, cpuHandle);

        RTVs[hash] = descriptorIndex;
        return cpuHandle;
    }

    return device->renderTargetViewHeap.GetCpuHandle(it->second);
}

/* D3D12ComputePassEncoder */
void D3D12ComputePassEncoder::Begin(const GPUComputePassDesc& desc)
{
    if (desc.label)
    {
        PushDebugGroup(desc.label);
        hasLabel = true;
    }
}

void D3D12ComputePassEncoder::EndEncoding()
{
    if (hasLabel)
    {
        PopDebugGroup();
    }

    commandBuffer->encoderActive = false;
    hasLabel = false;
}

void D3D12ComputePassEncoder::PushDebugGroup(const char* groupLabel) const
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void D3D12ComputePassEncoder::PopDebugGroup() const
{
    commandBuffer->PopDebugGroup();
}

void D3D12ComputePassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

void D3D12ComputePassEncoder::PrepareDispatch()
{

}

void D3D12ComputePassEncoder::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    PrepareDispatch();

    commandBuffer->commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void D3D12ComputePassEncoder::DispatchIndirect(GPUBuffer indirectBuffer, uint64_t indirectBufferOffset)
{

}

/* D3D12RenderPassEncoder */
void D3D12RenderPassEncoder::Begin(const GPURenderPassDesc& desc)
{
    if (desc.label)
    {
        PushDebugGroup(desc.label);
        hasLabel = true;
    }

    //Rect2D renderArea = { 0u, 0u, UINT32_MAX, UINT32_MAX };
    uint32_t width = UINT32_MAX;
    uint32_t height = UINT32_MAX;

    for (uint32_t i = 0; i < desc.colorAttachmentCount; ++i)
    {
        const GPURenderPassColorAttachment& attachment = desc.colorAttachments[i];
        if (!attachment.texture)
            continue;

        D3D12Texture* texture = static_cast<D3D12Texture*>(attachment.texture);
        const DXGI_FORMAT rtvFormat = ToDxgiRTVFormat(texture->desc.format);

        width = std::min(width, std::max(texture->desc.width >> attachment.mipLevel, 1u));
        height = std::min(height, std::max(texture->desc.height >> attachment.mipLevel, 1u));

        RTVs[i].cpuDescriptor = texture->GetRTV(rtvFormat, attachment.mipLevel);
        RTVs[i].BeginningAccess.Clear.ClearValue.Format = rtvFormat;

        const GPULoadAction loadAction = _ALIMER_DEF(attachment.loadAction, GPULoadAction_Discard);
        const GPUStoreAction storeAction = _ALIMER_DEF(attachment.storeAction, GPUStoreAction_Store);
        switch (loadAction)
        {
            default:
            case GPULoadAction_Load:
                RTVs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
                break;

            case GPULoadAction_Clear:
                RTVs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
                RTVs[i].BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor.r;
                RTVs[i].BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor.g;
                RTVs[i].BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor.b;
                RTVs[i].BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor.a;
                break;

            case GPULoadAction_Discard:
                RTVs[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
                break;
        }

        switch (storeAction)
        {
            default:
            case GPUStoreAction_Store:
                RTVs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
                break;

            case GPUStoreAction_Discard:
                RTVs[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
                break;
        }

        commandBuffer->TextureBarrier(texture, TextureLayout::RenderTarget);
    }

    const bool hasDepthOrStencil =
        desc.depthStencilAttachment != nullptr
        && desc.depthStencilAttachment->texture != nullptr;

    if (hasDepthOrStencil)
    {
    }

    commandBuffer->CommitBarriers();
    commandBuffer->commandList->BeginRenderPass(
        desc.colorAttachmentCount,
        RTVs,
        hasDepthOrStencil ? &DSV : nullptr,
        renderPassFlags
    );

    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, LONG(width), LONG(height) };
    commandBuffer->commandList->RSSetViewports(1, &viewport);
    commandBuffer->commandList->RSSetScissorRects(1, &scissorRect);
}

void D3D12RenderPassEncoder::EndEncoding()
{
    commandBuffer->commandList->EndRenderPass();

    if (hasLabel)
    {
        PopDebugGroup();
    }

    commandBuffer->encoderActive = false;
    hasLabel = false;
}

void D3D12RenderPassEncoder::PushDebugGroup(const char* groupLabel) const
{
    commandBuffer->PushDebugGroup(groupLabel);
}

void D3D12RenderPassEncoder::PopDebugGroup() const
{
    commandBuffer->PopDebugGroup();
}

void D3D12RenderPassEncoder::InsertDebugMarker(const char* markerLabel) const
{
    commandBuffer->InsertDebugMarker(markerLabel);
}

void D3D12RenderPassEncoder::SetViewport(const GPUViewport* viewport)
{
    commandBuffer->commandList->RSSetViewports(1, (const D3D12_VIEWPORT*)viewport);
}

void D3D12RenderPassEncoder::SetViewports(uint32_t viewportCount, const GPUViewport* viewports)
{
    ALIMER_ASSERT(viewports != nullptr);
    ALIMER_ASSERT(viewportCount < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);

    commandBuffer->commandList->RSSetViewports(viewportCount, (const D3D12_VIEWPORT*)viewports);
}

void D3D12RenderPassEncoder::SetScissorRect(const GPUScissorRect* scissorRect)
{
    D3D12_RECT d3dScissorRect;
    d3dScissorRect.left = LONG(scissorRect->x);
    d3dScissorRect.top = LONG(scissorRect->y);
    d3dScissorRect.right = LONG(scissorRect->x + scissorRect->width);
    d3dScissorRect.bottom = LONG(scissorRect->y + scissorRect->height);

    commandBuffer->commandList->RSSetScissorRects(1, &d3dScissorRect);
}

void D3D12RenderPassEncoder::SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects)
{
    ALIMER_ASSERT(scissorRects != nullptr);
    ALIMER_ASSERT(scissorCount < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);

    D3D12_RECT d3dScissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    for (uint32_t i = 0; i < scissorCount; ++i)
    {
        d3dScissorRects[i].left = LONG(scissorRects[i].x);
        d3dScissorRects[i].top = LONG(scissorRects[i].y);
        d3dScissorRects[i].right = LONG(scissorRects[i].x + scissorRects[i].width);
        d3dScissorRects[i].bottom = LONG(scissorRects[i].y + scissorRects[i].height);
    }

    commandBuffer->commandList->RSSetScissorRects(scissorCount, d3dScissorRects);
}

void D3D12RenderPassEncoder::SetStencilReference(uint32_t reference)
{
    commandBuffer->commandList->OMSetStencilRef(reference);
}

/* D3D12CommandBuffer */
D3D12CommandBuffer::~D3D12CommandBuffer()
{
    Clear();

    for (uint32_t i = 0; i < queue->device->maxFramesInFlight; ++i)
    {
        SAFE_RELEASE(commandAllocators[i]);
    }

    SAFE_RELEASE(commandList7);
    SAFE_RELEASE(commandList);

    delete computePassEncoder;
    delete renderPassEncoder;
}

void D3D12CommandBuffer::Clear()
{
    for (auto& surface : presentSurfaces)
    {
        surface->Release();
    }
    presentSurfaces.clear();
    globalBarriers.clear();
    textureBarriers.clear();
    bufferBarriers.clear();
    numBarriersToCommit = 0;

    //for (uint32_t i = 0; i < kMaxBindGroups; ++i)
    //{
    //    boundBindGroups[i].Reset();
    //}
}

void D3D12CommandBuffer::Begin(uint32_t frameIndex, const GPUCommandBufferDesc* desc)
{
    //GraphicsContext::Reset(frameIndex);
    //waits.clear();
    //hasPendingWaits.store(false);
    //currentPipeline.Reset();
    //currentPipelineLayout.Reset();
    //frameAllocators[frameIndex].Reset();
    Clear();

    // Start the command list in a default state:
    VHR(commandAllocators[frameIndex]->Reset());
    VHR(commandList->Reset(commandAllocators[frameIndex], nullptr));

    if (desc && desc->label)
    {
        PushDebugGroup(desc->label);
        hasLabel = true;
    }

#if TODO
    if (queue->queueType != GPUQueueType_Copy)
    {
        bindGroupsDirty = false;
        numBoundBindGroups = 0;

        for (uint32_t i = 0; i < kMaxBindGroups; ++i)
        {
            boundBindGroups[i].Reset();
        }

        ID3D12DescriptorHeap* heaps[2] = {
            device->shaderResourceViewHeap.GetShaderVisibleHeap(),
            device->samplerHeap.GetShaderVisibleHeap()
        };
        commandList->SetDescriptorHeaps(ALIMER_STATIC_ARRAY_SIZE(heaps), heaps);
    }
#endif // TODO

    if (queue->queueType == GPUQueueType_Graphics)
    {
        for (uint32_t i = 0; i < GPU_MAX_VERTEX_BUFFER_BINDINGS; ++i)
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
        commandList->RSSetScissorRects(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX, scissorRects);
        const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        commandList->OMSetBlendFactor(blendFactor);
    }

    encoderActive = false;
}

ID3D12CommandList* D3D12CommandBuffer::End()
{
    for (auto& surface : presentSurfaces)
    {
        TextureBarrier(surface->backbufferTextures[surface->backBufferIndex], TextureLayout::Present);
    }
    CommitBarriers();

    if (hasLabel)
    {
        PopDebugGroup();
    }

    VHR(commandList->Close());

    return commandList;
}

void D3D12CommandBuffer::TextureBarrier(const D3D12Texture* resource, TextureLayout newLayout, uint32_t subresource, bool commit)
{
    const uint32_t index = (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) ? 0 : subresource;
    const TextureLayout oldLayout = resource->subResourcesStates[index];

    if (queue->device->features.EnhancedBarriersSupported())
    {
        D3D12TextureLayoutMapping mappingBefore = ConvertTextureLayout(oldLayout);
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
        const D3D12_RESOURCE_STATES oldState = ConvertTextureLayoutLegacy(oldLayout);
        const D3D12_RESOURCE_STATES newState = ConvertTextureLayoutLegacy(newLayout);

        if (queue->queueType == GPUQueueType_Compute)
        {
            ALIMER_ASSERT((oldState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == oldState);
            ALIMER_ASSERT((newState & VALID_COMPUTE_QUEUE_RESOURCE_STATES) == newState);
        }

        if (oldState != newState)
        {
            //ALIMER_ASSERT_MSG(numBarriersToCommit < kMaxBarrierCount, "Exceeded arbitrary limit on buffered barriers");
            ALIMER_ASSERT(numBarriersToCommit < kMaxBarrierCount);

            D3D12_RESOURCE_BARRIER& barrier = barriers[numBarriersToCommit++];
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = resource->handle;
            barrier.Transition.Subresource = subresource; // D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = oldState;
            barrier.Transition.StateAfter = newState;

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
        else if (newState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            InsertUAVBarrier(resource, commit);
        }
    }

    if (commit || numBarriersToCommit == kMaxBarrierCount)
        CommitBarriers();
}

void D3D12CommandBuffer::InsertUAVBarrier(const D3D12Resource* resource, bool commit)
{
    ALIMER_ASSERT(numBarriersToCommit < kMaxBarrierCount);
    //ALIMER_ASSERT_MSG(numBarriersToCommit < kMaxBarrierCount, "Exceeded arbitrary limit on buffered barriers");

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
        commandList->ResourceBarrier(numBarriersToCommit, barriers);
    }

    numBarriersToCommit = 0;
}

GPUAcquireSurfaceResult D3D12CommandBuffer::AcquireSurfaceTexture(GPUSurface surface, GPUTexture* surfaceTexture)
{
    D3D12Surface* backendSurface = static_cast<D3D12Surface*>(surface);

    DWORD result = WaitForSingleObjectEx(
        backendSurface->frameLatencyWaitableObject,
        1000, // 1 second timeout (shouldn't ever occur)
        true
    );

    switch (result)
    {
        case WAIT_ABANDONED:
        case WAIT_FAILED:
            return GPUAcquireSurfaceResult_Lost;
        case WAIT_OBJECT_0:
            // OK
            break;
        case WAIT_TIMEOUT:
            return GPUAcquireSurfaceResult_SuccessOptimal;

        default:
            alimerLogError(LogCategory_GPU, "Unexpected wait status: 0x%08X", result);
            return GPUAcquireSurfaceResult_Lost;
    }

    // Fence and events?
    backendSurface->backBufferIndex = backendSurface->swapChain3->GetCurrentBackBufferIndex();
    D3D12Texture* currentTexture = backendSurface->backbufferTextures[backendSurface->backBufferIndex];
    *surfaceTexture = currentTexture;

    // Barrier
    backendSurface->AddRef();
    presentSurfaces.push_back(backendSurface);

    return GPUAcquireSurfaceResult_SuccessOptimal;
}

void D3D12CommandBuffer::PushDebugGroup(const char* groupLabel) const
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (d3d12_state.PIXBeginEventOnCommandList != nullptr)
    {
        d3d12_state.PIXBeginEventOnCommandList(commandList, PIX_COLOR_DEFAULT, groupLabel);
    }
    else
#endif
    {
        WCHAR* wideLabel = Win32_CreateWideStringFromUTF8(groupLabel);
        if (wideLabel)
        {
            PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, wideLabel);
            alimerFree(wideLabel);
        }
    }
}

void D3D12CommandBuffer::PopDebugGroup() const
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (d3d12_state.PIXEndEventOnCommandList != nullptr)
    {
        d3d12_state.PIXEndEventOnCommandList(commandList);
    }
    else
#endif
    {
        PIXEndEvent(commandList);
    }
}

void D3D12CommandBuffer::InsertDebugMarker(const char* markerLabel) const
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (d3d12_state.PIXSetMarkerOnCommandList != nullptr)
    {
        d3d12_state.PIXSetMarkerOnCommandList(commandList, PIX_COLOR_DEFAULT, markerLabel);
    }
    else
#endif
    {
        WCHAR* wideLabel = Win32_CreateWideStringFromUTF8(markerLabel);
        if (wideLabel)
        {
            PIXSetMarker(commandList, PIX_COLOR_DEFAULT, wideLabel);
            alimerFree(wideLabel);
        }
    }
}

GPUComputePassEncoder D3D12CommandBuffer::BeginComputePass(const GPUComputePassDesc& desc)
{
    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    computePassEncoder->Begin(desc);
    encoderActive = true;
    return computePassEncoder;
}

GPURenderPassEncoder D3D12CommandBuffer::BeginRenderPass(const GPURenderPassDesc& desc)
{
    if (encoderActive)
    {
        alimerLogError(LogCategory_GPU, "CommandEncoder already active");
        return nullptr;
    }

    renderPassEncoder->Begin(desc);
    encoderActive = true;
    return renderPassEncoder;
}

/* D3D12Queue */
GPUCommandBuffer D3D12Queue::AcquireCommandBuffer(const GPUCommandBufferDesc* desc)
{
    cmdBuffersLocker.lock();
    uint32_t index = cmdBuffersCount++;
    if (index >= commandBuffers.size())
    {
        D3D12CommandBuffer* commandBuffer = new D3D12CommandBuffer();
        commandBuffer->queue = this;
        commandBuffer->index = index;
        commandBuffer->computePassEncoder = new D3D12ComputePassEncoder();
        commandBuffer->computePassEncoder->commandBuffer = commandBuffer;
        commandBuffer->renderPassEncoder = new D3D12RenderPassEncoder();
        commandBuffer->renderPassEncoder->commandBuffer = commandBuffer;

        D3D12_COMMAND_LIST_TYPE d3dCommandListType = ToD3D12(queueType);

        for (uint32_t i = 0; i < device->maxFramesInFlight; ++i)
        {
            VHR(device->handle->CreateCommandAllocator(d3dCommandListType, PPV_ARGS(commandBuffer->commandAllocators[i])));
        }

        VHR(device->handle->CreateCommandList1(0, d3dCommandListType, D3D12_COMMAND_LIST_FLAG_NONE, PPV_ARGS(commandBuffer->commandList)));
        commandBuffer->commandList->QueryInterface(&commandBuffer->commandList7);

        commandBuffers.push_back(commandBuffer);
    }
    cmdBuffersLocker.unlock();

    commandBuffers[index]->Begin(device->frameIndex, desc);

    return commandBuffers[index];
}

uint64_t D3D12Queue::IncrementFenceValue()
{
    std::lock_guard<std::mutex> LockGuard(fenceMutex);
    handle->Signal(fence, nextFenceValue);
    return nextFenceValue++;
}

bool D3D12Queue::IsFenceComplete(uint64_t fenceValue)
{
    // Avoid querying the fence value by testing against the last one seen.
    // The max() is to protect against an unlikely race condition that could cause the last
    // completed fence value to regress.
    if (fenceValue > lastCompletedFenceValue)
    {
        lastCompletedFenceValue = std::max(lastCompletedFenceValue, fence->GetCompletedValue());
    }

    return fenceValue <= lastCompletedFenceValue;
}

void D3D12Queue::WaitForFenceValue(uint64_t fenceValue)
{
    if (IsFenceComplete(fenceValue))
        return;

    // NULL event handle will simply wait immediately:
    //	https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-seteventoncompletion#remarks
    fence->SetEventOnCompletion(fenceValue, nullptr);
    lastCompletedFenceValue = fenceValue;
}

void D3D12Queue::WaitIdle()
{
    WaitForFenceValue(IncrementFenceValue());
}

void D3D12Queue::Submit(uint32_t numCommandBuffers, GPUCommandBuffer const* commandBuffers)
{
    std::vector<ID3D12CommandList*> submitCommandLists;
    for (uint32_t i = 0; i < numCommandBuffers; i++)
    {
        D3D12CommandBuffer* commandBuffer = static_cast<D3D12CommandBuffer*>(commandBuffers[i]);

        ID3D12CommandList* commandList = commandBuffer->End();
        submitCommandLists.push_back(commandList);
    }

    // Kickoff the command lists
    handle->ExecuteCommandLists((UINT)submitCommandLists.size(), submitCommandLists.data());
    // Signal the next fence value (with the GPU)
    handle->Signal(fence, nextFenceValue++);

    //if (WaitForCompletion)
    //    g_CommandManager.WaitForFence(FenceValue);

    // Present surfaces
    for (uint32_t i = 0; i < numCommandBuffers; i++)
    {
        D3D12CommandBuffer* commandBuffer = static_cast<D3D12CommandBuffer*>(commandBuffers[i]);

        for (auto& surface : commandBuffer->presentSurfaces)
        {
            surface->Present();
            surface->Release();
        }

        commandBuffer->presentSurfaces.clear();
    }
}

/* D3D12CopyAllocator */
void D3D12CopyAllocator::Init(D3D12Device* device_)
{
    device = device_;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;
    VHR(device->handle->CreateCommandQueue(&queueDesc, PPV_ARGS(queue)));
    VHR(queue->SetName(L"CopyAllocator"));
}

void D3D12CopyAllocator::Shutdown()
{
    for (auto& context : freeList)
    {
        SAFE_RELEASE(context.commandAllocator);
        SAFE_RELEASE(context.commandList);
        SAFE_RELEASE(context.fence);
        SAFE_RELEASE(context.uploadBuffer);
        SAFE_RELEASE(context.uploadBufferAllocation);
        context.uploadBufferData = nullptr;
    }

    SAFE_RELEASE(queue);
}

D3D12UploadContext D3D12CopyAllocator::Allocate(uint64_t size)
{
    D3D12UploadContext context;

    locker.lock();

    // Try to search for a staging buffer that can fit the request:
    for (size_t i = 0; i < freeList.size(); ++i)
    {
        if (freeList[i].uploadBuffer != nullptr &&
            freeList[i].uploadBuffer->GetDesc().Width >= size)
        {
            if (freeList[i].IsCompleted())
            {
                VHR(freeList[i].fence->Signal(0));
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
        VHR(device->handle->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&context.commandAllocator)));
        VHR(device->handle->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, context.commandAllocator, nullptr, IID_PPV_ARGS(&context.commandList)));
        VHR(context.commandList->Close());

        VHR(device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context.fence)));
        SAFE_RELEASE(context.uploadBuffer);
        SAFE_RELEASE(context.uploadBufferAllocation);

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc{};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.Width = GetNextPowerOfTwo(size);
        bufferDesc.Height = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.Alignment = 0;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.SampleDesc.Quality = 0;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        VHR(device->allocator->CreateResource(
            &allocationDesc,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            &context.uploadBufferAllocation,
            IID_PPV_ARGS(&context.uploadBuffer)
        ));
        D3D12_RANGE readRange = {};
        VHR(context.uploadBuffer->Map(0, &readRange, &context.uploadBufferData));
    }

    // Begin command list
    VHR(context.commandAllocator->Reset());
    VHR(context.commandList->Reset(context.commandAllocator, nullptr));

    return context;
}

void D3D12CopyAllocator::Submit(D3D12UploadContext context)
{
    locker.lock();
    context.fenceValueSignaled++;
    freeList.push_back(context);
    locker.unlock();

    VHR(context.commandList->Close());
    ID3D12CommandList* commandLists[] = {
        context.commandList
    };

    queue->ExecuteCommandLists(1, commandLists);
    VHR(queue->Signal(context.fence, context.fenceValueSignaled));

    VHR(device->queues[GPUQueueType_Graphics].handle->Wait(context.fence, context.fenceValueSignaled));
    VHR(device->queues[GPUQueueType_Compute].handle->Wait(context.fence, context.fenceValueSignaled));
    VHR(device->queues[GPUQueueType_Copy].handle->Wait(context.fence, context.fenceValueSignaled));
    if (device->queues[GPUQueueType_VideoDecode].handle)
    {
        VHR(device->queues[GPUQueueType_VideoDecode].handle->Wait(context.fence, context.fenceValueSignaled));
    }
}

/* D3D12Device */
D3D12Device::~D3D12Device()
{
    WaitIdle();
    shuttingDown = true;

    // Shutdown copy allocator
    copyAllocator.Shutdown();

    // Shutdown descriptor heap allocators
    renderTargetViewHeap.Shutdown();
    depthStencilViewHeap.Shutdown();
    shaderResourceViewHeap.Shutdown();
    samplerHeap.Shutdown();

    // Destroy indirect command signatures
    SAFE_RELEASE(dispatchCommandSignature);

    // Destory pending objects.
    ProcessDeletionQueue(true);
    frameCount = 0;

    for (uint32_t queueIndex = 0; queueIndex < GPUQueueType_Count; ++queueIndex)
    {
        D3D12Queue& queue = queues[queueIndex];
        if (!queue.handle)
            continue;

        SAFE_RELEASE(queue.handle);
        SAFE_RELEASE(queue.fence);

        for (uint32_t frameIndex = 0; frameIndex < maxFramesInFlight; ++frameIndex)
        {
            SAFE_RELEASE(queue.frameFences[frameIndex]);
        }

        // Destroy command buffers
        for (size_t cmdBufferIndex = 0, count = queue.commandBuffers.size(); cmdBufferIndex < count; ++cmdBufferIndex)
        {
            delete queue.commandBuffers[cmdBufferIndex];
        }
        queue.commandBuffers.clear();
    }

    // Allocator.
    if (allocator != nullptr)
    {
        D3D12MA::TotalStatistics stats;
        allocator->CalculateStatistics(&stats);

        if (stats.Total.Stats.AllocationBytes > 0)
        {
            alimerLogWarn(LogCategory_GPU, "Total device memory leaked: %" PRId64 " bytes.", stats.Total.Stats.AllocationBytes);
        }

        allocator.Reset();
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    std::ignore = UnregisterWait(deviceRemovedWaitHandle);
    deviceRemovedFence.Reset();
#endif

    if (callbackCookie)
    {
        ComPtr<ID3D12InfoQueue1> infoQueue1 = nullptr;
        VHR(handle->QueryInterface(infoQueue1.GetAddressOf()));
        infoQueue1->UnregisterMessageCallback(callbackCookie);
        callbackCookie = 0;
    }

    videoDevice.Reset();
    //deviceConfiguration.Reset();
    const ULONG refCount = handle->Release();
#if defined(_DEBUG)
    if (refCount)
    {
        alimerLogDebug(LogCategory_GPU, "There are %d unreleased references left on the D3D device!", refCount);

        ID3D12DebugDevice* debugDevice = nullptr;
        if (SUCCEEDED(handle->QueryInterface(&debugDevice)))
        {
            debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
            debugDevice->Release();
        }
    }
#else
    (void)refCount; // avoid warning
#endif

    SAFE_RELEASE(adapter);
}

void D3D12Device::OnDeviceRemoved()
{

}

void D3D12Device::SetLabel(const char* label)
{
    WCHAR* wideLabel = Win32_CreateWideStringFromUTF8(label);
    if (wideLabel)
    {
        handle->SetName(wideLabel);
        alimerFree(wideLabel);
    }
}

bool D3D12Device::HasFeature(GPUFeature feature) const
{
    return adapter->HasFeature(feature);
}

GPUQueue D3D12Device::GetQueue(GPUQueueType type)
{
    return &queues[type];
}

bool D3D12Device::WaitIdle()
{
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        if (queues[i].handle == nullptr)
            continue;

        queues[i].WaitIdle();
    }

    ProcessDeletionQueue(true);
    return true;
}

uint64_t D3D12Device::CommitFrame()
{
    // Mark the completion of queues for this frame:
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        D3D12Queue& queue = queues[i];
        if (queue.handle == nullptr)
            continue;

        VHR(queue.handle->Signal(queue.frameFences[frameIndex], 1));
        queues[i].cmdBuffersCount = 0;
    }

    // Begin new frame
    frameCount++;
    frameIndex = frameCount % maxFramesInFlight;

    // Initiate stalling CPU when GPU is not yet finished with next frame
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        D3D12Queue& queue = queues[i];

        if (queue.handle == nullptr)
            continue;

        if (frameCount >= maxFramesInFlight
            && queue.frameFences[frameIndex]->GetCompletedValue() < 1)
        {
            // NULL event handle will simply wait immediately:
            //	https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-seteventoncompletion#remarks
            VHR(queue.frameFences[frameIndex]->SetEventOnCompletion(1, nullptr));
        }

        VHR(queue.frameFences[frameIndex]->Signal(0));
    }

    ProcessDeletionQueue(false);

    return frameCount;
}

ID3D12CommandSignature* D3D12Device::CreateCommandSignature(D3D12_INDIRECT_ARGUMENT_TYPE type, uint32_t stride)
{
    D3D12_INDIRECT_ARGUMENT_DESC argumentDesc{};
    argumentDesc.Type = type;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.ByteStride = stride;
    commandSignatureDesc.NumArgumentDescs = 1;
    commandSignatureDesc.pArgumentDescs = &argumentDesc;

    ID3D12CommandSignature* commandSignature = nullptr;
    HRESULT hr = handle->CreateCommandSignature(&commandSignatureDesc, nullptr, IID_PPV_ARGS(&commandSignature));
    if (FAILED(hr))
    {
        D3D12_LOG_ERROR(hr, "ID3D12Device::CreateCommandSignature() failed");
        return nullptr;
    }

    return commandSignature;
}

void D3D12Device::DeferDestroy(ID3D12DeviceChild* resource, D3D12MA::Allocation* allocation)
{
    if (resource == nullptr)
    {
        return;
    }

    if (shuttingDown)
    {
        resource->Release();
        SAFE_RELEASE(allocation);
        return;
    }

    destroyMutex.lock();
    deferredReleases.push_back(std::make_pair(resource, frameCount));
    if (allocation != nullptr)
    {
        deferredAllocations.push_back(std::make_pair(allocation, frameCount));
    }
    destroyMutex.unlock();
}

void D3D12Device::ProcessDeletionQueue(bool force)
{
    destroyMutex.lock();
    while (!deferredAllocations.empty())
    {
        if (force
            || (deferredAllocations.front().second + maxFramesInFlight < frameCount))
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
            || (deferredReleases.front().second + maxFramesInFlight < frameCount))
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

    destroyMutex.unlock();
}

GPUBuffer D3D12Device::CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData)
{
    D3D12Buffer* buffer = new D3D12Buffer();
    buffer->device = this;
    buffer->desc = desc;

    uint64_t alignedSize = desc.size;
    if (desc.usage & GPUBufferUsage_Constant)
    {
        alignedSize = AlignUp<uint64_t>(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    D3D12_RESOURCE_DESC1 resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.Width = alignedSize;
    resourceDesc.Height = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Alignment = 0;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (desc.usage & GPUBufferUsage_ShaderWrite)
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    if (!(desc.usage & GPUBufferUsage_ShaderRead)
        && !(desc.usage & GPUBufferUsage_RayTracing))
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

    D3D12_BARRIER_LAYOUT initialLayout = D3D12_BARRIER_LAYOUT_UNDEFINED;
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    if (desc.memoryType == GPUMemoryType_Readback)
    {
        allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
        initialLayout = D3D12_BARRIER_LAYOUT_COPY_DEST;
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

        buffer->immutableState = true;
    }
    else if (desc.memoryType == GPUMemoryType_Upload)
    {
        allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        initialLayout = D3D12_BARRIER_LAYOUT_GENERIC_READ;
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;

        buffer->immutableState = true;
    }
    else
    {
        buffer->immutableState = false;
    }

    buffer->currentState = initialState;

    HRESULT hr = E_FAIL;
    if (features.EnhancedBarriersSupported())
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
            initialState,
            nullptr,
            &buffer->allocation,
            IID_PPV_ARGS(&buffer->handle)
        );
    }

    if (FAILED(hr))
    {
        delete buffer;
        return nullptr;
    }

    if (desc.label)
    {
        buffer->SetLabel(desc.label);
    }

    handle->GetCopyableFootprints((D3D12_RESOURCE_DESC*)&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &buffer->allocatedSize);
    buffer->deviceAddress = buffer->handle->GetGPUVirtualAddress();

    if (desc.memoryType == GPUMemoryType_Readback)
    {
        VHR(buffer->handle->Map(0, nullptr, &buffer->pMappedData));
    }
    else if (desc.memoryType == GPUMemoryType_Upload)
    {
        D3D12_RANGE readRange = {};
        VHR(buffer->handle->Map(0, &readRange, &buffer->pMappedData));
    }

    // Issue data copy on request
    if (pInitialData != nullptr)
    {
        if (desc.memoryType == GPUMemoryType_Upload)
        {
            memcpy(buffer->pMappedData, pInitialData, desc.size);
        }
        else
        {
            D3D12UploadContext context = copyAllocator.Allocate(alignedSize);

            memcpy(context.uploadBufferData, pInitialData, desc.size);

            context.commandList->CopyBufferRegion(
                buffer->handle,
                0,
                context.uploadBuffer,
                0,
                desc.size
            );

            copyAllocator.Submit(context);
        }
    }

    return buffer;
}

GPUTexture D3D12Device::CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData)
{
    const bool isDepthStencil = alimerPixelFormatIsDepthStencil(desc.format);

    D3D12Texture* texture = new D3D12Texture();
    texture->device = this;
    texture->desc = desc;
    texture->dxgiFormat = (DXGI_FORMAT)alimerPixelFormatToDxgiFormat(desc.format);

    if (isDepthStencil && (desc.usage & (GPUBufferUsage_ShaderRead | GPUTextureUsage_ShaderWrite)))
    {
        texture->dxgiFormat = GetTypelessFormatFromDepthFormat(desc.format);
    }

    const uint32_t arraySizeMultiplier = (desc.dimension == TextureDimension_Cube) ? 6 : 1;
    D3D12_RESOURCE_DESC1 resourceDesc{};
    switch (desc.dimension)
    {
        case TextureDimension_1D:
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            resourceDesc.Width = desc.width;
            resourceDesc.Height = 1u;
            resourceDesc.DepthOrArraySize = (UINT16)desc.depthOrArrayLayers;
            break;

        case TextureDimension_2D:
        case TextureDimension_Cube:
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            resourceDesc.Width = desc.width;
            resourceDesc.Height = desc.height;
            resourceDesc.DepthOrArraySize = (UINT16)(desc.depthOrArrayLayers * arraySizeMultiplier);
            break;

        case TextureDimension_3D:
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            resourceDesc.Width = desc.width;
            resourceDesc.Height = desc.height;
            resourceDesc.DepthOrArraySize = (UINT16)desc.depthOrArrayLayers;
            break;
    }
    resourceDesc.MipLevels = (UINT16)desc.mipLevelCount;
    resourceDesc.Format = texture->dxgiFormat;
    resourceDesc.SampleDesc.Count = desc.sampleCount; // ToD3D12(desc.sampleCount);
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (desc.usage & GPUTextureUsage_RenderTarget)
    {
        if (isDepthStencil)
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            if (!(desc.usage & GPUTextureUsage_ShaderRead))
            {
                resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
            }
        }
        else
        {
            resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
    }

    if (desc.usage & GPUTextureUsage_ShaderWrite)
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    TextureLayout initialLayout = TextureLayout::Undefined;
    if (pInitialData == nullptr)
    {
        if (desc.usage & GPUTextureUsage_RenderTarget)
        {
            if (isDepthStencil)
            {
                initialLayout = TextureLayout::DepthWrite;
            }
            else
            {
                initialLayout = TextureLayout::RenderTarget;
            }
        }
        else if (desc.usage & GPUTextureUsage_ShaderWrite)
        {
            initialLayout = TextureLayout::UnorderedAccess;
        }
        else if (desc.usage & GPUTextureUsage_ShaderRead)
        {
            initialLayout = TextureLayout::ShaderResource;
        }
    }

    D3D12_CLEAR_VALUE clearValue = {};
    D3D12_CLEAR_VALUE* pClearValue = nullptr;

    if (desc.usage & GPUTextureUsage_RenderTarget)
    {
        clearValue.Format = resourceDesc.Format;
        if (isDepthStencil)
        {
            clearValue.DepthStencil.Depth = 0.0f; // Infinite reverse Z
        }
        pClearValue = &clearValue;
    }

    // If shader read/write and depth format, set to typeless
    if (isDepthStencil && (desc.usage & (GPUBufferUsage_ShaderRead | GPUTextureUsage_ShaderWrite)))
    {
        pClearValue = nullptr;
    }

    texture->allocatedSize = 0;
    texture->numSubResources = desc.mipLevelCount * desc.depthOrArrayLayers;
    texture->subResourcesStates.resize(texture->numSubResources);
    texture->footPrints.resize(texture->numSubResources);
    texture->rowSizesInBytes.resize(texture->footPrints.size());
    texture->numRows.resize(texture->footPrints.size());
    handle->GetCopyableFootprints(
        (D3D12_RESOURCE_DESC*)&resourceDesc,
        0,
        (UINT)texture->footPrints.size(),
        0,
        texture->footPrints.data(),
        texture->numRows.data(),
        texture->rowSizesInBytes.data(),
        &texture->allocatedSize
    );

    for (uint32_t i = 0; i < texture->numSubResources; i++)
    {
        texture->subResourcesStates[i] = initialLayout;
    }

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

    if (desc.usage & GPUTextureUsage_Shared)
    {
        // Dedicated memory
        allocationDesc.Flags = D3D12MA::ALLOCATION_FLAG_COMMITTED;

        // What about D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER and D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER?
        allocationDesc.ExtraHeapFlags |= D3D12_HEAP_FLAG_SHARED;
    }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (pInitialData != nullptr &&
        allocationDesc.HeapType == D3D12_HEAP_TYPE_DEFAULT &&
        features.CacheCoherentUMA() == TRUE
        )
    {
        // UMA: custom pool is like DEFAULT heap + CPU Write Combine
        // It will be used with WriteToSubresource to avoid GPU copy from UPLOAD to DEAFULT
        //allocationDesc.CustomPool = umaPool.Get();
    }
#endif

    D3D12TextureLayoutMapping textureLayout = ConvertTextureLayout(initialLayout);

    HRESULT hr = E_FAIL;
    if (features.EnhancedBarriersSupported())
    {
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
        allocator->CreateResource2(
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
        delete texture;
        alimerLogError(LogCategory_GPU, "D3D12: Failed to create texture");
        return nullptr;
    }

    if (desc.label)
    {
        texture->SetLabel(desc.label);
    }

    return texture;
}

GPUSampler D3D12Device::CreateSampler(const GPUSamplerDesc& desc)
{
    return nullptr;
}

GPUBindGroupLayout D3D12Device::CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc)
{
    return nullptr;
}

GPUPipelineLayout D3D12Device::CreatePipelineLayout(const GPUPipelineLayoutDesc& desc)
{
    return nullptr;
}

GPUShaderModule D3D12Device::CreateShaderModule(const GPUShaderModuleDesc* desc)
{
    return nullptr;
}

GPUComputePipeline D3D12Device::CreateComputePipeline(const GPUComputePipelineDesc& desc)
{
    return nullptr;
}

GPURenderPipeline D3D12Device::CreateRenderPipeline(const GPURenderPipelineDesc& desc)
{
    return nullptr;
}

/* D3D12Surface */
D3D12Surface::~D3D12Surface()
{
    Unconfigure();
}

GPUResult D3D12Surface::GetCapabilities(GPUAdapter adapter, GPUSurfaceCapabilities* capabilities) const
{
    capabilities->preferredFormat = PixelFormat_BGRA8UnormSrgb;
    capabilities->supportedUsage = GPUTextureUsage_ShaderRead | GPUTextureUsage_RenderTarget;
    static const PixelFormat kSupportedFormats[] = {
        PixelFormat_BGRA8Unorm,
        PixelFormat_BGRA8UnormSrgb,
        PixelFormat_RGBA8Unorm,
        PixelFormat_RGBA8UnormSrgb,
        PixelFormat_RGBA16Float,
        PixelFormat_RGB10A2Unorm,
    };
    capabilities->formats = kSupportedFormats;
    capabilities->formatCount = ALIMER_COUNT_OF(kSupportedFormats);
    return GPUResult_Success;
}

bool D3D12Surface::Configure(const GPUSurfaceConfig* config_)
{
    Unconfigure();

    // The range for `SetMaximumFrameLatency` is 1-16 so the maximum latency requested should be 15 because we add 1.
    // https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgidevice1-setmaximumframelatency
    //ALIMER_ASSERT(config_->desiredMaximumFrameLatency <= 15);

    config = *config_;
    device = static_cast<D3D12Device*>(config.device);
    device->AddRef();

    uint32_t maximumFrameLatency = device->maxFramesInFlight;

    // Nvidia recommends to use 1-2 more buffers than the maximum latency
    // https://developer.nvidia.com/blog/advanced-api-performance-swap-chains/
    // For high latency extra buffers seems excessive, so go with a minimum of 3 and beyond that add 1.
    uint32_t bufferCount = std::min(maximumFrameLatency + 1, 16u);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = config.width;
    swapChainDesc.Height = config.height;
    swapChainDesc.Format = ToDxgiSwapChainFormat(config.format);
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT |
        (instance->tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

    HRESULT hr = E_FAIL;
    ComPtr<IDXGISwapChain1> tempSwapChain;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
    fullscreenDesc.Windowed = TRUE; // !desc.fullscreen;

    hr = instance->dxgiFactory4->CreateSwapChainForHwnd(
        device->queues[GPUQueueType_Graphics].handle,
        handle,
        &swapChainDesc,
        &fullscreenDesc,
        nullptr,
        tempSwapChain.GetAddressOf()
    );

    // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
    VHR(instance->dxgiFactory4->MakeWindowAssociation(handle, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER));
#endif

    if (FAILED(hr))
    {
        return false;
    }

    hr = tempSwapChain->QueryInterface(&swapChain3);
    if (FAILED(hr))
    {
        return false;
    }

    VHR(swapChain3->SetMaximumFrameLatency(maximumFrameLatency));

    VHR(swapChain3->GetDesc1(&swapChainDesc));
    swapChainWidth = swapChainDesc.Width;
    swapChainHeight = swapChainDesc.Height;

    backBufferIndex = 0;
    backbufferTextures.resize(swapChainDesc.BufferCount);

    GPUTextureDesc textureDesc{};
    textureDesc.format = config.format;
    textureDesc.width = swapChainWidth;
    textureDesc.height = swapChainHeight;
    textureDesc.usage = GPUTextureUsage_ShaderRead | GPUTextureUsage_RenderTarget;

    for (uint32_t i = 0; i < swapChainDesc.BufferCount; ++i)
    {
        D3D12Texture* texture = new D3D12Texture();
        texture->device = device;
        texture->desc = textureDesc;
        texture->dxgiFormat = (DXGI_FORMAT)alimerPixelFormatToDxgiFormat(config.format);

        VHR(swapChain3->GetBuffer(i, IID_PPV_ARGS(&texture->handle)));

        D3D12_RESOURCE_DESC resourceDesc = texture->handle->GetDesc();

        texture->allocatedSize = 0;
        texture->numSubResources = resourceDesc.MipLevels * resourceDesc.DepthOrArraySize;
        texture->subResourcesStates.resize(texture->numSubResources);
        texture->subResourcesStates[0] = TextureLayout::Present;

        texture->footPrints.resize(texture->numSubResources);
        texture->rowSizesInBytes.resize(texture->footPrints.size());
        texture->numRows.resize(texture->footPrints.size());
        device->handle->GetCopyableFootprints(
            &resourceDesc,
            0,
            (UINT)texture->footPrints.size(),
            0,
            texture->footPrints.data(),
            texture->numRows.data(),
            texture->rowSizesInBytes.data(),
            &texture->allocatedSize
        );

        backbufferTextures[i] = texture;
    }

    backBufferIndex = swapChain3->GetCurrentBackBufferIndex();
    frameLatencyWaitableObject = swapChain3->GetFrameLatencyWaitableObject();

    return true;
}

void D3D12Surface::Unconfigure()
{
    if (device)
    {
        device->WaitIdle();
    }

    for (size_t i = 0; i < backbufferTextures.size(); ++i)
    {
        backbufferTextures[i]->Release();
    }

    swapChainWidth = 0;
    swapChainHeight = 0;
    backBufferIndex = 0;
    backbufferTextures.clear();
    if (frameLatencyWaitableObject != INVALID_HANDLE_VALUE)
    {
        CloseHandle(frameLatencyWaitableObject);
        frameLatencyWaitableObject = INVALID_HANDLE_VALUE;
    }

    SAFE_RELEASE(swapChain3);
    SAFE_RELEASE(device);
}

void D3D12Surface::Present()
{
    UINT syncInterval, presentFlags;
    switch (config.presentMode)
    {
        default:
        case GPUPresentMode_Fifo:
        case GPUPresentMode_FifoRelaxed:
            syncInterval = 1;
            presentFlags = 0;
            break;

        case GPUPresentMode_Immediate:
            syncInterval = 0;
            presentFlags = instance->tearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0;
            break;

        case GPUPresentMode_Mailbox:
            syncInterval = 0;
            presentFlags = 0;
            break;
    }

    const HRESULT hr = swapChain3->Present(syncInterval, presentFlags);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
#ifdef _DEBUG
        char buff[64] = {};
        sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? device->handle->GetDeviceRemovedReason() : hr));
        OutputDebugStringA(buff);
#endif

        // Handle device lost
        device->OnDeviceRemoved();
    }
}

/* D3D12Adapter */
D3D12Adapter::~D3D12Adapter()
{
    alimerFree(deviceName);
}

GPUResult D3D12Adapter::GetInfo(GPUAdapterInfo* info) const
{
    memset(info, 0, sizeof(GPUAdapterInfo));

    info->deviceName = deviceName;
    memcpy(info->driverVersion, driverVersion, sizeof(uint16_t) * 4);
    info->driverDescription = driverDescription.c_str();
    info->adapterType = adapterType;
    info->vendor = agpuGPUAdapterVendorFromID(vendorID);
    info->vendorID = vendorID;
    info->deviceID = deviceID;

    return GPUResult_Success;
}

GPUResult D3D12Adapter::GetLimits(GPULimits* limits) const
{
    limits->maxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
    limits->maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
    limits->maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
    limits->maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
    limits->maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
    // Max number of "constants" where each constant is a 16-byte float4
    limits->maxConstantBufferBindingSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
    limits->maxStorageBufferBindingSize = (1 << D3D12_REQ_BUFFER_RESOURCE_TEXEL_COUNT_2_TO_EXP) - 1;
    limits->minConstantBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    limits->minStorageBufferOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT;
    limits->maxBufferSize = D3D12_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM * 1024ull * 1024ull;
    limits->maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
    limits->maxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    limits->viewportBoundsMin = D3D12_VIEWPORT_BOUNDS_MIN;
    limits->viewportBoundsMax = D3D12_VIEWPORT_BOUNDS_MAX;

    // https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-downlevel-compute-shaders
    // Thread Group Shared Memory is limited to 16Kb on downlevel hardware. This is less than
    // the 32Kb that is available to Direct3D 11 hardware. D3D12 is also 32kb.
    limits->maxComputeWorkgroupStorageSize = 32768;
    limits->maxComputeInvocationsPerWorkgroup = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;

    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/sm5-attributes-numthreads
    limits->maxComputeWorkgroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
    limits->maxComputeWorkgroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
    limits->maxComputeWorkgroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_dispatch_arguments
    limits->maxComputeWorkgroupsPerDimension = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;

    //
    limits->conservativeRasterizationTier = conservativeRasterizationTier;

    return GPUResult_Success;
}

bool D3D12Adapter::HasFeature(GPUFeature feature) const
{
    switch (feature)
    {
        // Always supported features
        case GPUFeature_DepthClipControl:
        case GPUFeature_Depth32FloatStencil8:
        case GPUFeature_TimestampQuery:
        case GPUFeature_PipelineStatisticsQuery:
        case GPUFeature_TextureCompressionBC:
        case GPUFeature_IndirectFirstInstance:
        case GPUFeature_DualSourceBlending:
            return true;

            // Never supported features
        case GPUFeature_TextureCompressionETC2:
        case GPUFeature_TextureCompressionASTC:
        case GPUFeature_TextureCompressionASTC_HDR:
            return false;

        case GPUFeature_ShaderFloat16:
            return shaderFloat16;

        case GPUFeature_GPUUploadHeapSupported:
            return GPUUploadHeapSupported;

        case GPUFeature_CopyQueueTimestampQueriesSupported:
            return copyQueueTimestampQueriesSupported;

        case GPUFeature_CacheCoherentUMA:
            return cacheCoherentUMA;

        case GPUFeature_ShaderOutputViewportIndex:
            return shaderOutputViewportIndex;

        case GPUFeature_ConservativeRasterization:
            return conservativeRasterizationTier != GPUConservativeRasterizationTier_NotSupported;

        default:
            return false;
    }
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
inline void HandleDeviceRemoved(PVOID context, BOOLEAN)
{
    D3D12Device* removedDevice = (D3D12Device*)context;
    removedDevice->OnDeviceRemoved();
}
#endif

GPUDevice D3D12Adapter::CreateDevice(const GPUDeviceDesc& desc)
{
    D3D12Device* device = new D3D12Device();
    device->adapter = this;
    device->maxFramesInFlight = desc.maxFramesInFlight;

    HRESULT hr = E_FAIL;
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    if (d3d12_state.deviceFactory != nullptr)
    {
        hr = d3d12_state.deviceFactory->CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device->handle));
    }
    else
#endif
    {
        hr = d3d12_D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device->handle));
    }

    if (FAILED(hr))
    {
        VHR(hr);
        return nullptr;
    }

    if (desc.label)
    {
        device->SetLabel(desc.label);
    }

    if (SUCCEEDED(device->handle->QueryInterface(device->videoDevice.ReleaseAndGetAddressOf())))
    {
    }

    // Init feature check (https://devblogs.microsoft.com/directx/introducing-a-new-api-for-checking-feature-support-in-direct3d-12/)
    VHR(device->features.Init(device->handle));

    if (instance->validationMode != GPUValidationMode_Disabled)
    {
        // Configure debug device (if active).
        ComPtr<ID3D12InfoQueue> infoQueue;
        if (SUCCEEDED(device->handle->QueryInterface(infoQueue.GetAddressOf())))
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

            if (instance->validationMode == GPUValidationMode_Verbose)
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

            VHR(infoQueue->AddStorageFilterEntries(&filter));
        }

        ComPtr<ID3D12InfoQueue1> infoQueue1 = nullptr;
        if (SUCCEEDED(device->handle->QueryInterface(infoQueue1.GetAddressOf())))
        {
            infoQueue1->RegisterMessageCallback(DebugMessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &device->callbackCookie);
        }
    }

    // Create fence to detect device removal
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    {
        VHR(device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&device->deviceRemovedFence)));

        HANDLE deviceRemovedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        VHR(device->deviceRemovedFence->SetEventOnCompletion(UINT64_MAX, deviceRemovedEvent));

        RegisterWaitForSingleObject(
            &device->deviceRemovedWaitHandle,
            deviceRemovedEvent,
            HandleDeviceRemoved,
            this, // Pass the device as our context
            INFINITE, // No timeout
            0 // No flags
        );
    }
#endif

    // Create command queues
    for (uint32_t queue = 0; queue < GPUQueueType_Count; ++queue)
    {
        GPUQueueType queueType = (GPUQueueType)queue;
        if (queueType >= GPUQueueType_VideoDecode && device->videoDevice == nullptr)
            continue;

        device->queues[queue].device = device;
        device->queues[queue].queueType = queueType;

        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = ToD3D12(queueType);
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        VHR(device->handle->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&device->queues[queue].handle)));
        VHR(device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&device->queues[queue].fence)));
        VHR(device->queues[queue].fence->Signal((uint64_t)queueDesc.Type << 56));
        device->queues[queue].nextFenceValue = ((uint64_t)queueDesc.Type << 56 | 1);
        device->queues[queue].lastCompletedFenceValue = (uint64_t)queueDesc.Type << 56;

        switch (queueType)
        {
            case GPUQueueType_Graphics:
                device->queues[queue].handle->SetName(L"Graphics Queue");
                device->queues[queue].fence->SetName(L"GraphicsQueue - Fence");
                break;
            case GPUQueueType_Compute:
                device->queues[queue].handle->SetName(L"Compute Queue");
                device->queues[queue].fence->SetName(L"ComputeQueue - Fence");
                break;
            case GPUQueueType_Copy:
                device->queues[queue].handle->SetName(L"CopyQueue");
                device->queues[queue].fence->SetName(L"CopyQueue - Fence");
                break;
            case GPUQueueType_VideoDecode:
                device->queues[queue].handle->SetName(L"VideoDecode");
                device->queues[queue].fence->SetName(L"VideoDecode - Fence");
                break;
            default:
                break;
        }

        // Create frame-resident resources:
        for (uint32_t frameIndex = 0; frameIndex < device->maxFramesInFlight; ++frameIndex)
        {
            VHR(
                device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&device->queues[queue].frameFences[frameIndex]))
            );

#if defined(_DEBUG)
            wchar_t fenceName[64];

            switch (queueType)
            {
                case GPUQueueType_Graphics:
                    swprintf(fenceName, 64, L"GraphicsQueue - Frame Fence %u", frameIndex);
                    break;
                case GPUQueueType_Compute:
                    swprintf(fenceName, 64, L"ComputeQueue - Frame Fence %u", frameIndex);
                    break;
                case GPUQueueType_Copy:
                    swprintf(fenceName, 64, L"CopyQueue - Frame Fence %u", frameIndex);
                    break;
                case GPUQueueType_VideoDecode:
                    swprintf(fenceName, 64, L"VideoDecode - Frame Fence %u", frameIndex);
                    break;
                default:
                    break;
            }

            device->queues[queue].frameFences[frameIndex]->SetName(fenceName);
#endif
        }
    }

    // Create allocator
    D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.pDevice = device->handle;
    allocatorDesc.pAdapter = dxgiAdapter1.Get();
    allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED;
    allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED;
    allocatorDesc.Flags |= D3D12MA::ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED;
    //allocatorDesc.PreferredBlockSize = VMA_PREFERRED_BLOCK_SIZE;

    if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &device->allocator)))
    {
        return nullptr;
    }

    // Create indirect command signatures
    device->dispatchCommandSignature = device->CreateCommandSignature(D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH, sizeof(D3D12_DISPATCH_ARGUMENTS));

    // Init copy/upload allocatr
    device->copyAllocator.Init(device);

    // Init CPU/GPU descriptor allocators
    const uint32_t renderTargetViewHeapSize = 1024;
    const uint32_t depthStencilViewHeapSize = 256;

    // Maximum number of CBV/SRV/UAV descriptors in heap for Tier 1
    const uint32_t shaderResourceViewHeapSize = 1000000;

    // Maximum number of samplers descriptors in heap for Tier 1
    const uint32_t samplerHeapSize = 2048; // 2048 ->  Tier1 limit

    device->renderTargetViewHeap.Init(device->handle, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetViewHeapSize);
    device->depthStencilViewHeap.Init(device->handle, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthStencilViewHeapSize);

    // Shader visible descriptor heaps
    device->shaderResourceViewHeap.Init(device->handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceViewHeapSize);
    device->samplerHeap.Init(device->handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerHeapSize);

    return device;
}

/* D3D12Instance */
D3D12Instance::~D3D12Instance()
{
}

GPUSurface D3D12Instance::CreateSurface(Window* window)
{
    D3D12Surface* surface = new D3D12Surface();
    surface->instance = this;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    HWND hwnd = static_cast<HWND>(alimerWindowGetNativeHandle(window));
    if (!IsWindow(hwnd))
    {
        alimerLogError(LogCategory_GPU, "Win32: Invalid vulkan hwnd handle");
        return nullptr;
    }
    surface->handle = hwnd;

    RECT windowRect;
    GetClientRect(hwnd, &windowRect);
    surface->width = static_cast<uint32_t>(windowRect.right - windowRect.left);
    surface->height = static_cast<uint32_t>(windowRect.bottom - windowRect.top);
#endif

    return surface;
}

GPUAdapter D3D12Instance::RequestAdapter(const GPURequestAdapterOptions* options)
{
    const DXGI_GPU_PREFERENCE gpuPreference = (options && options->powerPreference == GPUPowerPreference_LowPower) ? DXGI_GPU_PREFERENCE_MINIMUM_POWER : DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;

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

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<ID3D12Device> tempDevice;
    for (uint32_t i = 0; NextAdapter(i, dxgiAdapter1.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 adapterDesc;
        VHR(dxgiAdapter1->GetDesc1(&adapterDesc));

        // Don't select the Basic Render Driver adapter.
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        if (d3d12_state.deviceFactory != nullptr)
        {
            if (SUCCEEDED(d3d12_state.deviceFactory->CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(tempDevice.ReleaseAndGetAddressOf()))))
            {
                break;
            }
        }
        else
#endif
        {
            if (SUCCEEDED(d3d12_D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(tempDevice.ReleaseAndGetAddressOf()))))
            {
                break;
            }
        }
    }

    ALIMER_ASSERT(dxgiAdapter1 != nullptr);
    if (dxgiAdapter1 == nullptr)
    {
        alimerLogWarn(LogCategory_GPU, "D3D12: No capable adapter found!");
        return nullptr;
    }

    CD3DX12FeatureSupport features;
    VHR(features.Init(tempDevice.Get()));

    D3D12Adapter* adapter = new D3D12Adapter();
    adapter->instance = this;
    adapter->dxgiAdapter1 = dxgiAdapter1;

    DXGI_ADAPTER_DESC1 adapterDesc;
    VHR(dxgiAdapter1->GetDesc1(&adapterDesc));

    adapter->deviceName = Win32_CreateUTF8FromWideString(adapterDesc.Description);
    // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
    LARGE_INTEGER umdVersion;
    if (dxgiAdapter1->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
    {
        uint64_t encodedVersion = umdVersion.QuadPart;
        uint16_t mask = 0xFFFF;

        std::ostringstream o;
        o << "D3D12 driver version ";

        for (size_t i = 0; i < 4; ++i)
        {
            adapter->driverVersion[i] = (encodedVersion >> (48 - 16 * i)) & mask;
            o << adapter->driverVersion[i] << ".";
        }

        adapter->driverDescription = o.str();
    }

    adapter->vendorID = adapterDesc.VendorId;
    adapter->deviceID = adapterDesc.DeviceId;

    // Detect adapter type.
    if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
        adapter->adapterType = GPUAdapterType_CPU;
    }
    else
    {
        adapter->adapterType = features.UMA() ? GPUAdapterType_IntegratedGPU : GPUAdapterType_DiscreteGPU;
    }

    //const bool supportsDP4a = d3dFeatures.HighestShaderModel() >= D3D_SHADER_MODEL_6_4;
    adapter->shaderFloat16 = features.HighestShaderModel() >= D3D_SHADER_MODEL_6_2 && features.Native16BitShaderOpsSupported();
    adapter->GPUUploadHeapSupported = features.GPUUploadHeapSupported() == TRUE;
    adapter->copyQueueTimestampQueriesSupported = features.CopyQueueTimestampQueriesSupported() == TRUE;
    adapter->cacheCoherentUMA = features.CacheCoherentUMA() == TRUE;
    adapter->shaderOutputViewportIndex = features.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation() == TRUE;
    adapter->conservativeRasterizationTier = static_cast<GPUConservativeRasterizationTier>(features.ConservativeRasterizationTier());

    return adapter;
}

bool D3D12_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;
    // Linux:  const char* libName = "libdxvk_dxgi.so";

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    d3d12_state.lib_dxgi = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    d3d12_state.lib_d3d12 = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (d3d12_state.lib_dxgi == nullptr || d3d12_state.lib_d3d12 == nullptr)
    {
        return false;
    }

    d3d12_state.CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(d3d12_state.lib_dxgi, "CreateDXGIFactory2");
    if (d3d12_state.CreateDXGIFactory2 == nullptr)
    {
        return false;
    }

#if defined(_DEBUG)
    d3d12_state.DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)GetProcAddress(d3d12_state.lib_dxgi, "DXGIGetDebugInterface1");
#endif

    // Use new D3D12GetInterface and agility SDK
    static PFN_D3D12_GET_INTERFACE func_D3D12GetInterface = (PFN_D3D12_GET_INTERFACE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12GetInterface");
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
                if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, agilitySdkPath.c_str(), IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()))))
                {
                    func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()));
                }
                else if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, ".\\", IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()))))
                {
                    func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()));
                }
            }
        }
    }

    if (!d3d12_state.deviceFactory)
    {
        d3d12_state.D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12CreateDevice");
        if (!d3d12_state.D3D12CreateDevice)
        {
            return false;
        }

        d3d12_state.D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12GetDebugInterface");
        d3d12_state.D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12SerializeVersionedRootSignature");
        if (!d3d12_state.D3D12SerializeVersionedRootSignature) {
            return false;
        }
    }

    // Try to load PIX (WinPixEventRuntime.dll)
    d3d12_state.lib_WinPixEventRuntime = LoadLibraryW(L"WinPixEventRuntime.dll");
    if (d3d12_state.lib_WinPixEventRuntime != nullptr)
    {
        d3d12_state.PIXBeginEventOnCommandList = (PFN_PIXBeginEventOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXBeginEventOnCommandList");
        d3d12_state.PIXEndEventOnCommandList = (PFN_PIXEndEventOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXEndEventOnCommandList");
        d3d12_state.PIXSetMarkerOnCommandList = (PFN_PIXSetMarkerOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXSetMarkerOnCommandList");
    }
#endif

    ComPtr<IDXGIFactory4> dxgiFactory;
    if (FAILED(dxgi_CreateDXGIFactory2(0, PPV_ARGS(dxgiFactory))))
    {
        return false;
    }

    ComPtr<IDXGIAdapter1> dxgiAdapter;
    bool foundCompatibleDevice = false;
    for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, dxgiAdapter.ReleaseAndGetAddressOf()); ++i)
    {
        DXGI_ADAPTER_DESC1 adapterDesc;
        VHR(dxgiAdapter->GetDesc1(&adapterDesc));

        // Don't select the Basic Render Driver adapter.
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
        if (d3d12_state.deviceFactory != nullptr)
        {
            if (SUCCEEDED(d3d12_state.deviceFactory->CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
            {
                foundCompatibleDevice = true;
                break;
            }
        }
        else
        {
            if (SUCCEEDED(d3d12_D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
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

    available = false;
    return false;
}

GPUInstance* D3D12_CreateInstance(const GPUConfig* config)
{
    D3D12Instance* instance = new D3D12Instance();
    instance->validationMode = config->validationMode;

    HRESULT hr = E_FAIL;
    DWORD dxgiFactoryFlags = 0;
    if (config->validationMode != GPUValidationMode_Disabled)
    {
        dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

        ComPtr<ID3D12Debug> debugController;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        if (d3d12_state.deviceFactory)
        {
            hr = d3d12_state.deviceFactory->GetConfigurationInterface(CLSID_D3D12Debug_Alimer, IID_PPV_ARGS(debugController.GetAddressOf()));
        }
        else
#endif
        {
            hr = d3d12_D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));
        }

        if (SUCCEEDED(hr))
        {
            debugController->EnableDebugLayer();

            if (config->validationMode == GPUValidationMode_GPU)
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
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            if (d3d12_state.deviceFactory)
            {
                hr = d3d12_state.deviceFactory->GetConfigurationInterface(CLSID_D3D12DeviceRemovedExtendedData_Alimer, IID_PPV_ARGS(pDredSettings.GetAddressOf()));
            }
            else
#endif
            {
                hr = d3d12_D3D12GetDebugInterface(IID_PPV_ARGS(pDredSettings.GetAddressOf()));
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
            if (SUCCEEDED(dxgi_DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
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
    VHR(dxgi_CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&instance->dxgiFactory4)));

    BOOL allowTearing = FALSE;
    ComPtr<IDXGIFactory5> dxgiFactory5;
    hr = instance->dxgiFactory4.As(&dxgiFactory5);
    if (SUCCEEDED(hr))
    {
        hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    }

    if (FAILED(hr) || !allowTearing)
    {
        instance->tearingSupported = false;
    }
    else
    {
        instance->tearingSupported = true;
    }

    return instance;
}

#undef VHR

#endif /* defined(ALIMER_GPU_D3D12) */
