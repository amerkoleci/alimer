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

#include <vector>
#include <mutex>

using Microsoft::WRL::ComPtr;

#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
    | D3D12_RESOURCE_STATE_COPY_DEST \
    | D3D12_RESOURCE_STATE_COPY_SOURCE )

namespace
{

}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);

const GUID CLSID_D3D12SDKConfiguration_Alimer = { 0x7cda6aca, 0xa03e, 0x49c8, {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce} };
const GUID CLSID_D3D12DeviceFactory_Alimer = { 0x114863bf, 0xc386, 0x4aee, {0xb3, 0x9d, 0x8f, 0x0b, 0xbb, 0x06, 0x29, 0x55} };
[[maybe_unused]] const GUID CLSID_D3D12Debug_Alimer = { 0xf2352aeb, 0xdd84, 0x49fe, {0xb9, 0x7b, 0xa9, 0xdc, 0xfd, 0xcc, 0x1b, 0x4f} };
[[maybe_unused]] const GUID CLSID_D3D12DeviceRemovedExtendedData_Alimer = { 0x4a75bbc4, 0x9ff4, 0x4ad8, {0x9f, 0x18, 0xab, 0xae, 0x84, 0xdc, 0x5f, 0xf2} };

struct D3D12_State {
    HMODULE lib_dxgi;
    HMODULE lib_d3d12;
    
    PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

#if defined(_DEBUG)
    PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;
#endif

    PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

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
    }
} d3d12_state;

#define dxgi_CreateDXGIFactory2 d3d12_state.CreateDXGIFactory2
#else
#define dxgi_CreateDXGIFactory2 CreateDXGIFactory2
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
    ID3D12CommandQueue* handle = nullptr;
    //VkFence frameFences[GPU_MAX_INFLIGHT_FRAMES] = {};
    //std::mutex mutex;

    std::vector<D3D12GPUCommandBuffer*> commandBuffers;
    uint32_t cmdBuffersCount = 0;
    std::mutex cmdBuffersLocker;

    GPUCommandBuffer CreateCommandBuffer(const GPUCommandBufferDescriptor* descriptor) override;
    void Submit();
};

struct D3D12GPUDevice final : public GPUDeviceImpl
{
    D3D12GPUAdapter* adapter = nullptr;
    ID3D12Device5* handle = nullptr;
    D3D12GPUQueue queues[GPUQueueType_Count];
    D3D12MA::Allocator* allocator = nullptr;

    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;

    ~D3D12GPUDevice() override;
    GPUQueue GetQueue(GPUQueueType type) override;
    uint64_t CommitFrame() override;
    void ProcessDeletionQueue();

    /* Resource creation */
    GPUBuffer CreateBuffer(const GPUBufferDescriptor* descriptor, const void* pInitialData) override;
};

struct D3D12GPUSurface final : public GPUSurfaceImpl
{
    ~D3D12GPUSurface() override;
};

struct D3D12GPUAdapter final : public GPUAdapterImpl
{
    D3D12GPUInstance* instance = nullptr;
    IDXGIAdapter1* handle = nullptr;

    GPUResult GetLimits(GPULimits* limits) const override;
    GPUDevice CreateDevice() override;
};

struct D3D12GPUInstance final : public GPUInstance
{
    IDXGIFactory4* handle = nullptr;

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
GPUCommandBuffer D3D12GPUQueue::CreateCommandBuffer(const GPUCommandBufferDescriptor* descriptor)
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

void D3D12GPUQueue::Submit()
{
    if (handle == nullptr)
        return;
}

/* D3D12GPUDevice */
D3D12GPUDevice::~D3D12GPUDevice()
{
    //WaitIdle();
    //shuttingDown = true;
}

GPUQueue D3D12GPUDevice::GetQueue(GPUQueueType type)
{
    return &queues[type];
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

    ProcessDeletionQueue();

    return frameCount;
}

void D3D12GPUDevice::ProcessDeletionQueue()
{

}

GPUBuffer D3D12GPUDevice::CreateBuffer(const GPUBufferDescriptor* descriptor, const void* pInitialData)
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

GPUDevice D3D12GPUAdapter::CreateDevice()
{
    D3D12GPUDevice* device = new D3D12GPUDevice();
    device->adapter = this;

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
    return surface;
}

GPUAdapter D3D12GPUInstance::RequestAdapter(const GPURequestAdapterOptions* options)
{
    return nullptr;
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
#endif

    ComPtr<IDXGIFactory4> dxgiFactory;
    if (FAILED(dxgi_CreateDXGIFactory2(0, PPV_ARGS(dxgiFactory))))
    {
        return false;
    }

    available = true;
    return true;
}

GPUInstance* D3D12_CreateInstance(const GPUConfig* config)
{
    D3D12GPUInstance* instance = new D3D12GPUInstance();
    return instance;
}

#endif /* defined(ALIMER_GPU_D3D12) */
