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
#   include <directx/d3dx12_resource_helpers.h>
#   include <directx/d3dx12_pipeline_state_stream.h>
#   include <directx/d3dx12_check_feature_support.h>
#   include <dcomp.h>
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

#define SAFE_RELEASE(obj) do \
{ \
  if ((obj)) { \
    (obj)->Release(); \
    (obj) = nullptr; \
  } \
} while(0)

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
#include <mutex>
#include <vector>
#include <deque>

using Microsoft::WRL::ComPtr;

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace
{
    template <typename T>
    static T AlignUp(T val, T alignment)
    {
        //D3D12MA_HEAVY_ASSERT(IsPow2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }

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

struct D3D12GPUInstance;
struct D3D12GPUAdapter;
struct D3D12Queue;
struct D3D12GPUDevice;

struct D3D12Resource
{
    D3D12GPUDevice* device = nullptr;
    ID3D12Resource* handle = nullptr;
    D3D12MA::Allocation* allocation = nullptr;
    bool immutableState = false;
    uint32_t numSubResources = 0;
    mutable std::vector<D3D12_RESOURCE_STATES> subResourcesStates;
};

struct D3D12Buffer final : public GPUBuffer, public D3D12Resource
{
    GPUBufferDesc desc;
    uint64_t allocatedSize = 0;
    D3D12_GPU_VIRTUAL_ADDRESS deviceAddress = 0;
    void* pMappedData = nullptr;
    HANDLE sharedHandle = nullptr;

    ~D3D12Buffer() override;
    void SetLabel(const char* label) override;
    uint64_t GetSize() const override { return desc.size; }
    GPUDeviceAddress GetDeviceAddress() const override { return deviceAddress; }
};

struct D3D12Texture final : public GPUTexture, public D3D12Resource
{
    HANDLE sharedHandle = nullptr;

    ~D3D12Texture() override;
    void SetLabel(const char* label) override;
};

struct D3D12CommandBuffer final : public GPUCommandBuffer
{
    D3D12Queue* queue = nullptr;
    uint32_t index = 0;
};

struct D3D12Queue final : public GPUQueue
{
    D3D12GPUDevice* device = nullptr;
    GPUQueueType type = GPUQueueType_Count;
    ComPtr<ID3D12CommandQueue> handle;
    ComPtr<ID3D12Fence> fence;
    uint64_t nextFenceValue = 0;
    uint64_t lastCompletedFenceValue = 0;
    std::mutex fenceMutex;
    ComPtr<ID3D12Fence> frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};

    std::vector<D3D12CommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUQueueType GetType() const override { return type; }
    GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) override;

    uint64_t IncrementFenceValue();
    bool IsFenceComplete(uint64_t fenceValue);
    void WaitForFenceValue(uint64_t fenceValue);
    void WaitIdle();
    void Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers) override;
};

struct D3D12UploadContext final
{
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
    ID3D12Fence* fence = nullptr;
    uint64_t fenceValueSignaled = 0;
    D3D12Buffer* uploadBuffer = nullptr;
    void* uploadBufferData = nullptr;

    inline bool IsValid() const { return commandList != nullptr; }
    inline bool IsCompleted() const { return fence->GetCompletedValue() >= fenceValueSignaled; }
};

struct D3D12CopyAllocator final
{
    D3D12GPUDevice* device = nullptr;
    // Separate copy queue to reduce interference with main copy queue.
    ID3D12CommandQueue* queue = nullptr;
    std::mutex locker;
    std::vector<D3D12UploadContext> freeList;

    void Init(D3D12GPUDevice* device);
    void Shutdown();
    D3D12UploadContext Allocate(uint64_t size);
    void Submit(D3D12UploadContext context);
};

struct D3D12GPUDevice final : public GPUDevice
{
    D3D12GPUAdapter* adapter = nullptr;
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

    ~D3D12GPUDevice() override;
    void OnDeviceRemoved();
    GPUQueue* GetQueue(GPUQueueType type) override;
    bool WaitIdle() override;
    uint64_t CommitFrame() override;
    void DeferDestroy(ID3D12DeviceChild* resource, D3D12MA::Allocation* allocation = nullptr);
    void ProcessDeletionQueue(bool force);

    /* Resource creation */
    GPUBuffer* CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData) override;
    GPUTexture* CreateTexture(const GPUTextureDesc* desc, const void* pInitialData) override;
};

struct D3D12Surface final : public GPUSurface
{
    D3D12GPUInstance* instance = nullptr;
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    HWND handle = nullptr;
#endif
    uint32_t width = 0;
    uint32_t height = 0;

    ~D3D12Surface() override;
    void Configure(const GPUSurfaceConfiguration* config) override;
    void Unconfigure() override;
};

struct D3D12GPUAdapter final : public GPUAdapter
{
    D3D12GPUInstance* instance = nullptr;
    ComPtr<IDXGIAdapter1> dxgiAdapter1;

    GPUResult GetLimits(GPULimits* limits) const override;
    GPUDevice* CreateDevice() override;
};

struct D3D12GPUInstance final : public GPUInstance
{
    ComPtr<IDXGIFactory4> dxgiFactory4;
    bool tearingSupported = false;
    GPUValidationMode validationMode;

    ~D3D12GPUInstance() override;
    GPUSurface* CreateSurface(Window* window) override;
    GPUAdapter* RequestAdapter(const GPURequestAdapterOptions* options) override;
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

/* D3D12Queue */
GPUCommandBuffer* D3D12Queue::AcquireCommandBuffer(const GPUCommandBufferDesc* desc)
{
    cmdBuffersLocker.lock();
    uint32_t index = cmdBuffersCount++;
    if (index >= commandBuffers.size())
    {
        D3D12CommandBuffer* commandBuffer = new D3D12CommandBuffer();
        commandBuffer->queue = this;
        commandBuffer->index = index;
        commandBuffers.push_back(commandBuffer);
    }
    cmdBuffersLocker.unlock();

    //commandBuffers[index]->Begin(frameIndex, label);

    return commandBuffers[index];
}

uint64_t D3D12Queue::IncrementFenceValue()
{
    std::lock_guard<std::mutex> LockGuard(fenceMutex);
    handle->Signal(fence.Get(), nextFenceValue);
    return nextFenceValue++;
}

bool D3D12Queue::IsFenceComplete(uint64_t fenceValue)
{
    // Avoid querying the fence value by testing against the last one seen.
    // The max() is to protect against an unlikely race condition that could cause the last
    // completed fence value to regress.
    if (fenceValue > lastCompletedFenceValue)
    {
        lastCompletedFenceValue = ALIMER_MAX(lastCompletedFenceValue, fence->GetCompletedValue());
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

void D3D12Queue::Submit(uint32_t numCommandBuffers, GPUCommandBuffer* const* commandBuffers)
{
}

/* D3D12CopyAllocator */
void D3D12CopyAllocator::Init(D3D12GPUDevice* device_)
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
            freeList[i].uploadBuffer->GetSize() >= size)
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

        GPUBufferDesc uploadBufferDesc;
        uploadBufferDesc.label = "CopyAllocator::UploadBuffer";
        uploadBufferDesc.size = GetNextPowerOfTwo(size);
        uploadBufferDesc.memoryType = GPUMemoryType_Upload;

        context.uploadBuffer = static_cast<D3D12Buffer*>(device->CreateBuffer(&uploadBufferDesc, nullptr));
        ALIMER_ASSERT(context.uploadBuffer != nullptr);
        context.uploadBufferData = context.uploadBuffer->pMappedData;
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

/* D3D12GPUDevice */
D3D12GPUDevice::~D3D12GPUDevice()
{
    WaitIdle();
    shuttingDown = true;

    copyAllocator.Shutdown();

    // Destory pending objects.
    ProcessDeletionQueue(true);
    frameCount = 0;

    for (uint8_t i = 0; i < GPUQueueType_Count; ++i)
    {
        queues[i].handle.Reset();
        queues[i].fence.Reset();

        for (uint32_t frameIndex = 0; frameIndex < GPU_MAX_INFLIGHT_FRAMES; ++frameIndex)
        {
            queues[i].frameFences[frameIndex].Reset();
        }
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

    //videoDevice.Reset();
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
}

void D3D12GPUDevice::OnDeviceRemoved()
{

}

GPUQueue* D3D12GPUDevice::GetQueue(GPUQueueType type)
{
    return &queues[type];
}

bool D3D12GPUDevice::WaitIdle()
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

uint64_t D3D12GPUDevice::CommitFrame()
{
    // Final submits with fences.
    for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
    {
        //queues[i].Submit(queues[i].frameFences[frameIndex]);
    }

    // Begin new frame
    frameCount++;
    frameIndex = frameCount % GPU_MAX_INFLIGHT_FRAMES;

    // Initiate stalling CPU when GPU is not yet finished with next frame
    if (frameCount >= GPU_MAX_INFLIGHT_FRAMES)
    {
        for (uint32_t i = 0; i < GPUQueueType_Count; ++i)
        {
            if (queues[i].handle == nullptr)
                continue;

            //VK_CHECK(vkWaitForFences(handle, 1, &queues[i].frameFences[frameIndex], true, 0xFFFFFFFFFFFFFFFF));
            //VK_CHECK(vkResetFences(handle, 1, &queues[i].frameFences[frameIndex]));
        }
    }

    ProcessDeletionQueue(false);

    return frameCount;
}

void D3D12GPUDevice::DeferDestroy(ID3D12DeviceChild* resource, D3D12MA::Allocation* allocation)
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

void D3D12GPUDevice::ProcessDeletionQueue(bool force)
{
    destroyMutex.lock();
    while (!deferredAllocations.empty())
    {
        if (force
            || (deferredAllocations.front().second + GPU_MAX_INFLIGHT_FRAMES < frameCount))
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
            || (deferredReleases.front().second + GPU_MAX_INFLIGHT_FRAMES < frameCount))
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

GPUBuffer* D3D12GPUDevice::CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData)
{
    D3D12Buffer* buffer = new D3D12Buffer();
    buffer->device = this;
    buffer->desc = *desc;

    uint64_t alignedSize = desc->size;
    if (desc->usage & GPUBufferUsage_Constant)
    {
        alignedSize = AlignUp<uint64_t>(alignedSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.Width = alignedSize;
    resourceDesc.Height = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Alignment = 0;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (desc->usage & GPUBufferUsage_ShaderWrite)
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    if (!(desc->usage & GPUBufferUsage_ShaderRead)
        && !(desc->usage & GPUBufferUsage_RayTracing))
    {
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
    }

    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;

    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

    D3D12MA::ALLOCATION_DESC allocationDesc = {};
    allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
    if (desc->memoryType == GPUMemoryType_Readback)
    {
        allocationDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
        initialState = D3D12_RESOURCE_STATE_COPY_DEST;
        resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

        buffer->immutableState = true;
    }
    else if (desc->memoryType == GPUMemoryType_Upload)
    {
        allocationDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
        initialState = D3D12_RESOURCE_STATE_GENERIC_READ;

        buffer->immutableState = true;
    }
    else
    {
        buffer->immutableState = false;
    }

    buffer->numSubResources = 1;
    buffer->subResourcesStates.resize(1);
    buffer->subResourcesStates[0] = initialState;

    HRESULT hr = E_FAIL;
    const bool isSparse = false;
    if (isSparse)
        //if (CheckBitsAny(desc.usage, RHIBufferUsage::Sparse))
    {
        hr = handle->CreateReservedResource(
            &resourceDesc,
            initialState,
            nullptr,
            IID_PPV_ARGS(&buffer->handle)
        );
        //buffer->sparsePageSize = D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
    }
    else
    {
        hr = allocator->CreateResource(
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

    if (desc->label)
    {
        buffer->SetLabel(desc->label);
    }

    handle->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &buffer->allocatedSize);
    buffer->deviceAddress = buffer->handle->GetGPUVirtualAddress();

    if (desc->memoryType == GPUMemoryType_Readback)
    {
        VHR(buffer->handle->Map(0, nullptr, &buffer->pMappedData));
    }
    else if (desc->memoryType == GPUMemoryType_Upload)
    {
        D3D12_RANGE readRange = {};
        VHR(buffer->handle->Map(0, &readRange, &buffer->pMappedData));
    }

    // Issue data copy on request
    if (pInitialData != nullptr)
    {
        if (desc->memoryType == GPUMemoryType_Upload)
        {
            memcpy(buffer->pMappedData, pInitialData, desc->size);
        }
        else
        {
            D3D12UploadContext context = copyAllocator.Allocate(alignedSize);

            memcpy(context.uploadBufferData, pInitialData, desc->size);

            context.commandList->CopyBufferRegion(
                buffer->handle,
                0,
                context.uploadBuffer->handle,
                0,
                desc->size
            );

            copyAllocator.Submit(context);
        }
    }

    return buffer;
}

GPUTexture* D3D12GPUDevice::CreateTexture(const GPUTextureDesc* desc, const void* pInitialData)
{
    return nullptr;
}

/* D3D12Surface */
D3D12Surface::~D3D12Surface()
{
}

void D3D12Surface::Configure(const GPUSurfaceConfiguration* config)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = config->width;
    swapChainDesc.Height = config->height;
    swapChainDesc.Format = ToDxgiSwapChainFormat(config->format);
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; // PresentModeToBufferCount(desc.presentMode);
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | (instance->tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
}

void D3D12Surface::Unconfigure()
{

}

/* D3D12GPUAdapter */
GPUResult D3D12GPUAdapter::GetLimits(GPULimits* limits) const
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

    return GPUResult_Success;
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
inline void HandleDeviceRemoved(PVOID context, BOOLEAN)
{
    D3D12GPUDevice* removedDevice = (D3D12GPUDevice*)context;
    removedDevice->OnDeviceRemoved();
}
#endif

GPUDevice* D3D12GPUAdapter::CreateDevice()
{
    D3D12GPUDevice* device = new D3D12GPUDevice();
    device->adapter = this;

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

        device->queues[queue].type = queueType;

        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = ToD3D12(queueType);
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        VHR(
            device->handle->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(device->queues[queue].handle.ReleaseAndGetAddressOf()))
        );
        VHR(
            device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(device->queues[queue].fence.ReleaseAndGetAddressOf()))
        );
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
        for (uint32_t frameIndex = 0; frameIndex < GPU_MAX_INFLIGHT_FRAMES; ++frameIndex)
        {
            VHR(
                device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(device->queues[queue].frameFences[frameIndex].ReleaseAndGetAddressOf()))
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

    // Init copy/upload allocatr
    device->copyAllocator.Init(device);

    return device;
}

/* D3D12GPUInstance */
D3D12GPUInstance::~D3D12GPUInstance()
{
}

GPUSurface* D3D12GPUInstance::CreateSurface(Window* window)
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

GPUAdapter* D3D12GPUInstance::RequestAdapter(const GPURequestAdapterOptions* options)
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
            if (SUCCEEDED(d3d12_state.deviceFactory->CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
        else
#endif
        {
            if (SUCCEEDED(d3d12_D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
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

    D3D12GPUAdapter* adapter = new D3D12GPUAdapter();
    adapter->instance = this;
    adapter->dxgiAdapter1 = dxgiAdapter1;
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
    D3D12GPUInstance* instance = new D3D12GPUInstance();
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
