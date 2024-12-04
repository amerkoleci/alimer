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
    inline void __stdcall DebugMessageCallback(
        D3D12_MESSAGE_CATEGORY Category,
        D3D12_MESSAGE_SEVERITY Severity,
        D3D12_MESSAGE_ID ID,
        LPCSTR pDescription,
        [[maybe_unused]] void* pContext)
    {
        if (Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION || Severity == D3D12_MESSAGE_SEVERITY_ERROR)
        {
            alimerLogError(LogCategory_GPU, "%s", pDescription);
        }
        else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
        {
            alimerLogWarn(LogCategory_GPU, "%s", pDescription);
        }
        else
        {
            alimerLogInfo(LogCategory_GPU, "%s", pDescription);
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

                //case GPUQueueType_VideoDecode:
                //    return D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE;

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
[[maybe_unused]] const GUID CLSID_D3D12Debug_Alimer = { 0xf2352aeb, 0xdd84, 0x49fe, {0xb9, 0x7b, 0xa9, 0xdc, 0xfd, 0xcc, 0x1b, 0x4f} };
[[maybe_unused]] const GUID CLSID_D3D12DeviceRemovedExtendedData_Alimer = { 0x4a75bbc4, 0x9ff4, 0x4ad8, {0x9f, 0x18, 0xab, 0xae, 0x84, 0xdc, 0x5f, 0xf2} };

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
struct D3D12GPUQueue;
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

struct D3D12GPUBuffer final : public GPUBufferImpl, public D3D12Resource
{
    uint64_t allocatedSize = 0;
    D3D12_GPU_VIRTUAL_ADDRESS deviceAddress = 0;
    void* pMappedData = nullptr;
    HANDLE sharedHandle = nullptr;

    ~D3D12GPUBuffer() override;
    void SetLabel([[maybe_unused]] const char* label) override;
};

struct D3D12GPUCommandBuffer final : public GPUCommandBufferImpl
{
    D3D12GPUQueue* queue = nullptr;
    uint32_t index = 0;
};

struct D3D12GPUQueue final : public GPUQueueImpl
{
    D3D12GPUDevice* device = nullptr;
    ComPtr<ID3D12CommandQueue> handle;
    ComPtr<ID3D12Fence> fence;
    uint64_t nextFenceValue = 0;
    uint64_t lastCompletedFenceValue = 0;
    std::mutex fenceMutex;
    ComPtr<ID3D12Fence> frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};

    std::vector<D3D12GPUCommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUCommandBuffer CreateCommandBuffer(const GPUCommandBufferDesc* desc) override;

    uint64_t IncrementFenceValue();
    bool IsFenceComplete(uint64_t fenceValue);
    void WaitForFenceValue(uint64_t fenceValue);
    void WaitIdle();
    void Submit();
};

struct D3D12GPUDevice final : public GPUDeviceImpl
{
    D3D12GPUAdapter* adapter = nullptr;
    ID3D12Device5* handle = nullptr;
    CD3DX12FeatureSupport features;
    DWORD callbackCookie = 0;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    //ComPtr<D3D12MA::Pool> umaPool;
    ComPtr<ID3D12Fence> deviceRemovedFence;
    HANDLE deviceRemovedWaitHandle = nullptr;
#endif

    D3D12GPUQueue queues[GPUQueueType_Count];
    ComPtr<D3D12MA::Allocator> allocator;

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
    GPUQueue GetQueue(GPUQueueType type) override;
    bool WaitIdle() override;
    uint64_t CommitFrame() override;
    void ProcessDeletionQueue(bool force);

    /* Resource creation */
    GPUBuffer CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData) override;
    GPUTexture CreateTexture(const GPUTextureDesc* desc, const void* pInitialData) override;
};

struct D3D12GPUSurface final : public GPUSurfaceImpl
{
    HWND handle = nullptr;
    ~D3D12GPUSurface() override;
};

struct D3D12GPUAdapter final : public GPUAdapterImpl
{
    D3D12GPUInstance* instance = nullptr;
    ComPtr<IDXGIAdapter1> dxgiAdapter1;

    GPUResult GetLimits(GPULimits* limits) const override;
    GPUDevice CreateDevice() override;
};

struct D3D12GPUInstance final : public GPUInstance
{
    ComPtr<IDXGIFactory4> dxgiFactory4;
    bool tearingSupported = false;
    GPUValidationMode validationMode;

    ~D3D12GPUInstance() override;
    GPUSurface CreateSurface(Window* window) override;
    GPUAdapter RequestAdapter(const GPURequestAdapterOptions* options) override;
};

/* D3D12GPUBuffer */
D3D12GPUBuffer::~D3D12GPUBuffer()
{

}

void D3D12GPUBuffer::SetLabel(const char* label)
{
    //auto wName = ToUtf16(label);
    //handle->SetName(wName.c_str());
    //allocation->SetName(wName.c_str());
}

/* D3D12GPUQueue */
GPUCommandBuffer D3D12GPUQueue::CreateCommandBuffer(const GPUCommandBufferDesc* desc)
{
    cmdBuffersLocker.lock();
    uint32_t index = cmdBuffersCount++;
    if (index >= commandBuffers.size())
    {
        D3D12GPUCommandBuffer* commandBuffer = new D3D12GPUCommandBuffer();
        commandBuffer->queue = this;
        commandBuffer->index = index;
        commandBuffers.push_back(commandBuffer);
    }
    cmdBuffersLocker.unlock();

    //commandBuffers[index]->Begin(frameIndex, label);

    return commandBuffers[index];
}

uint64_t D3D12GPUQueue::IncrementFenceValue()
{
    std::lock_guard<std::mutex> LockGuard(fenceMutex);
    handle->Signal(fence.Get(), nextFenceValue);
    return nextFenceValue++;
}

bool D3D12GPUQueue::IsFenceComplete(uint64_t fenceValue)
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

void D3D12GPUQueue::WaitForFenceValue(uint64_t fenceValue)
{
    if (IsFenceComplete(fenceValue))
        return;

    // NULL event handle will simply wait immediately:
    //	https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12fence-seteventoncompletion#remarks
    fence->SetEventOnCompletion(fenceValue, nullptr);
    lastCompletedFenceValue = fenceValue;
}

void D3D12GPUQueue::WaitIdle()
{
    WaitForFenceValue(IncrementFenceValue());
}

void D3D12GPUQueue::Submit()
{
    if (handle == nullptr)
        return;
}

/* D3D12GPUDevice */
D3D12GPUDevice::~D3D12GPUDevice()
{
    WaitIdle();

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
            //alimerLogWarn(LogCategory_GPU, "Total device memory leaked: {} bytes.", stats.Total.Stats.AllocationBytes);
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
        ThrowIfFailed(handle->QueryInterface(infoQueue1.GetAddressOf()));
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

GPUQueue D3D12GPUDevice::GetQueue(GPUQueueType type)
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

GPUBuffer D3D12GPUDevice::CreateBuffer(const GPUBufferDesc* desc, const void* pInitialData)
{
    return nullptr;
}

GPUTexture D3D12GPUDevice::CreateTexture(const GPUTextureDesc* desc, const void* pInitialData)
{
    return nullptr;
}

/* D3D12GPUSurface */
D3D12GPUSurface::~D3D12GPUSurface()
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

    return GPUResult_Success;
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
inline void HandleDeviceRemoved(PVOID context, BOOLEAN)
{
    D3D12GPUDevice* removedDevice = (D3D12GPUDevice*)context;
    removedDevice->OnDeviceRemoved();
}
#endif

GPUDevice D3D12GPUAdapter::CreateDevice()
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
        ThrowIfFailed(hr);
        return nullptr;
    }

    // Init feature check (https://devblogs.microsoft.com/directx/introducing-a-new-api-for-checking-feature-support-in-direct3d-12/)
    ThrowIfFailed(device->features.Init(device->handle));

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

            ThrowIfFailed(infoQueue->AddStorageFilterEntries(&filter));
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
        ThrowIfFailed(device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&device->deviceRemovedFence)));

        HANDLE deviceRemovedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        ThrowIfFailed(device->deviceRemovedFence->SetEventOnCompletion(UINT64_MAX, deviceRemovedEvent));

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
#if TODO_VIDEO_DECODE
        if (queueType >= GPUQueueType_VideoDecode && videoDevice == nullptr)
            continue;
#endif

        D3D12_COMMAND_QUEUE_DESC queueDesc{};
        queueDesc.Type = ToD3D12(queueType);
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;
        ThrowIfFailed(
            device->handle->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(device->queues[queue].handle.ReleaseAndGetAddressOf()))
        );
        ThrowIfFailed(
            device->handle->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(device->queues[queue].fence.ReleaseAndGetAddressOf()))
        );
        ThrowIfFailed(device->queues[queue].fence->Signal((uint64_t)queueDesc.Type << 56));
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
#if TODO_VIDEO_DECODE
            case GPUQueueType_VideoDecode:
                device->queues[queue].handle->SetName(L"VideoDecode");
                device->queues[queue].fence->SetName(L"VideoDecode - Fence");
                break;
#endif
            default:
                break;
        }

        // Create frame-resident resources:
        for (uint32_t frameIndex = 0; frameIndex < GPU_MAX_INFLIGHT_FRAMES; ++frameIndex)
        {
            ThrowIfFailed(
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
                    //case GPUQueueType_VideoDecode:
                    //    swprintf(fenceName, 64, L"VideoDecode - Frame Fence %u", frameIndex);
                    //    break;
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

    return device;
}

/* D3D12GPUInstance */
D3D12GPUInstance::~D3D12GPUInstance()
{
}

GPUSurface D3D12GPUInstance::CreateSurface(Window* window)
{
    HWND hwnd = static_cast<HWND>(alimerWindowGetNativeHandle(window));
    if (!IsWindow(hwnd))
    {
        alimerLogError(LogCategory_GPU, "Win32: Invalid vulkan hwnd handle");
        return nullptr;
    }

    D3D12GPUSurface* surface = new D3D12GPUSurface();
    surface->handle = hwnd;
    return surface;
}

GPUAdapter D3D12GPUInstance::RequestAdapter(const GPURequestAdapterOptions* options)
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
        ThrowIfFailed(dxgiAdapter1->GetDesc1(&adapterDesc));

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
        ThrowIfFailed(dxgiAdapter->GetDesc1(&adapterDesc));

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
    ThrowIfFailed(dxgi_CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&instance->dxgiFactory4)));

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

#endif /* defined(ALIMER_GPU_D3D12) */
