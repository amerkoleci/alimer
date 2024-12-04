// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_GPU_WEBGPU)
#   include "alimer_gpu_internal.h"
#if defined(__EMSCRIPTEN__)
#   include <emscripten.h>
#   include <emscripten/html5_webgpu.h>
#else
#   define WGPU_SKIP_DECLARATIONS
#   include "third_party/webgpu/webgpu.h"
#   include "third_party/webgpu/wgpu.h"

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

// clang-format off
#define ALIMER_RHI_WGPU_PROCS(x) \
    x(CreateInstance)\
    x(GetProcAddress)\
    x(InstanceCreateSurface)\
    x(InstanceHasWGSLLanguageFeature)\
    x(InstanceProcessEvents)\
    x(InstanceRequestAdapter)\
    x(InstanceReference) \
    x(InstanceRelease) \
    x(AdapterEnumerateFeatures) \
    x(AdapterGetInfo) \
    x(AdapterGetLimits) \
    x(AdapterHasFeature) \
    x(AdapterRequestDevice) \
    x(AdapterReference) \
    x(AdapterRelease) \
    x(AdapterInfoFreeMembers) \
    x(DeviceCreateCommandEncoder) \
    x(DeviceDestroy) \
    x(DeviceEnumerateFeatures) \
    x(DeviceGetLimits) \
    x(DeviceGetQueue) \
    x(DeviceHasFeature) \
    x(DevicePopErrorScope) \
    x(DevicePushErrorScope) \
    x(DeviceSetLabel) \
    x(DeviceReference) \
    x(DeviceRelease) \
    x(QueueOnSubmittedWorkDone) \
    x(QueueSetLabel) \
    x(QueueSubmit) \
    x(QueueWriteBuffer) \
    x(QueueWriteTexture) \
    x(QueueReference) \
    x(QueueRelease) \
    x(CommandBufferSetLabel) \
    x(CommandBufferReference) \
    x(CommandBufferRelease) \
    x(CommandEncoderBeginComputePass) \
    x(CommandEncoderBeginRenderPass) \
    x(CommandEncoderClearBuffer) \
    x(CommandEncoderCopyBufferToBuffer) \
    x(CommandEncoderCopyBufferToTexture) \
    x(CommandEncoderCopyTextureToBuffer) \
    x(CommandEncoderCopyTextureToTexture) \
    x(CommandEncoderFinish) \
    x(CommandEncoderInsertDebugMarker) \
    x(CommandEncoderPopDebugGroup) \
    x(CommandEncoderPushDebugGroup) \
    x(CommandEncoderResolveQuerySet) \
    x(CommandEncoderWriteTimestamp) \
    x(CommandEncoderReference) \
    x(CommandEncoderRelease) \
    x(SurfaceConfigure) \
    x(SurfaceGetCapabilities) \
    x(SurfaceGetCurrentTexture) \
    x(SurfacePresent) \
    x(SurfaceSetLabel) \
    x(SurfaceUnconfigure) \
    x(SurfaceReference) \
    x(SurfaceRelease) \
    x(SurfaceCapabilitiesFreeMembers) \
    x(TextureCreateView) \
    x(TextureDestroy) \
    x(TextureSetLabel) \
    x(TextureReference) \
    x(TextureRelease)

#define WGPU_DECLARE_PROC(name) WGPUProc##name wgpu##name;
ALIMER_RHI_WGPU_PROCS(WGPU_DECLARE_PROC);

struct WGPU_State
{
#if defined(_WIN32)
    HMODULE wgpu_module = nullptr;
#else
    void* wgpu_module = nullptr;
#endif
    bool dawn = false;

    ~WGPU_State()
    {
        if (wgpu_module)
        {
#if defined(_WIN32)
            FreeLibrary(wgpu_module);
#else
            dlclose(wgpu_module);
#endif
            wgpu_module = nullptr;
        }
    }
} wgpu_state;
#endif

namespace
{
    constexpr WGPUPowerPreference ToWGPU(GPUPowerPreference value)
    {
        switch (value)
        {
            case GPUPowerPreference_LowPower:
                return WGPUPowerPreference_LowPower;
            case GPUPowerPreference_HighPerformance:
                return WGPUPowerPreference_HighPerformance;

            case GPUPowerPreference_Undefined:
            default:
                return WGPUPowerPreference_Undefined;
        }
    }

    const char* ToString(WGPUDeviceLostReason value)
    {
        switch (value)
        {
            default:
            case WGPUDeviceLostReason_Unknown:
                return "Unknown";
            case WGPUDeviceLostReason_Destroyed:
                return "Destroyed";
        }
    }

    static void DeviceLostCallback(WGPUDeviceLostReason reason, char const* message, void* userdata)
    {
        ALIMER_UNUSED(userdata);

        alimerLogError(LogCategory_GPU, "WGPU device lost: reason %s - %s", ToString(reason), message);
    }

    static void UncapturedErrorCallback(WGPUErrorType type, char const* message, void* userdata)
    {
        ALIMER_UNUSED(userdata);

        switch (type)
        {
            case WGPUErrorType_NoError:
                alimerLogError(LogCategory_GPU, "WGPU: %s", message);
                break;
            case WGPUErrorType_Validation:
                alimerLogError(LogCategory_GPU, "WGPU Validation: %s", message);
                break;
            case WGPUErrorType_OutOfMemory:
                alimerLogError(LogCategory_GPU, "WGPU OutOfMemory: %s", message);
                break;
            case WGPUErrorType_Internal:
                alimerLogError(LogCategory_GPU, "WGPU Internal: %s", message);
                break;
            case WGPUErrorType_Unknown:
                alimerLogError(LogCategory_GPU, "WGPU Unknown: %s", message);
                break;
            case WGPUErrorType_DeviceLost:
                alimerLogError(LogCategory_GPU, "WGPU DeviceLost: %s", message);
                break;
            default:
                break;
        }
    }
}

struct WebGPUDevice final : public GPUDeviceImpl
{
    WGPUDevice handle = nullptr;

    uint64_t frameCount = 0;
    uint32_t frameIndex = 0;

    ~WebGPUDevice() override;
    GPUQueue GetQueue(GPUQueueType type) override;
    uint64_t CommitFrame() override;
    void SetLabel(const char* label) override;

    /* Resource creation */
    GPUBuffer CreateBuffer(const GPUBufferDescriptor* descriptor, const void* pInitialData) override;
};

struct WebGPUSurface final : public GPUSurfaceImpl
{
    WGPUSurface handle = nullptr;

    ~WebGPUSurface() override;
    void SetLabel(const char* label) override;
};

struct WebGPUAdapter final : public GPUAdapterImpl
{
    WGPUAdapter handle = nullptr;

    ~WebGPUAdapter() override;
    GPUResult GetLimits(GPULimits* limits) const override;
    GPUDevice CreateDevice() override;
};

struct WebGPUInstance final : public GPUInstance
{
    WGPUInstance handle = nullptr;

    ~WebGPUInstance() override;

    GPUSurface CreateSurface(Window* window) override;
    GPUAdapter RequestAdapter(const GPURequestAdapterOptions* options) override;
};

/* WebGPUDevice */
WebGPUDevice::~WebGPUDevice()
{
}

GPUQueue WebGPUDevice::GetQueue(GPUQueueType type)
{
    ALIMER_UNUSED(type);
    return nullptr;
}

uint64_t WebGPUDevice::CommitFrame()
{
#if !defined(__EMSCRIPTEN__)
    if (wgpu_state.dawn)
#endif
    {
        //wgpuInstanceProcessEvents(handle);
    }

    return 0;
}

void WebGPUDevice::SetLabel(const char* label)
{
    wgpuDeviceSetLabel(handle, label);
}

GPUBuffer WebGPUDevice::CreateBuffer(const GPUBufferDescriptor* descriptor, const void* pInitialData)
{
    ALIMER_UNUSED(descriptor);
    ALIMER_UNUSED(pInitialData);

    return nullptr;
}

/* WebGPUSurface */
WebGPUSurface::~WebGPUSurface()
{
}

void WebGPUSurface::SetLabel(const char* label)
{
    wgpuSurfaceSetLabel(handle, label);
}

/* WebGPUAdapter */
WebGPUAdapter::~WebGPUAdapter()
{
    wgpuAdapterRelease(handle);
}

GPUResult WebGPUAdapter::GetLimits(GPULimits* limits) const
{
    // TODO
    ALIMER_UNUSED(limits);
    return GPUResult_Success;
}

GPUDevice WebGPUAdapter::CreateDevice()
{
    struct UserData {
        WGPUDevice device = nullptr;
        bool requestEnded = false;
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        }
        else
        {
            alimerLogError(LogCategory_GPU, "WebGPU: Could not get device: %s", message);
        }
        userData.requestEnded = true;
    };

    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    //deviceDesc.label = desc.label.empty() ? nullptr : desc.label.data();
    //deviceDesc.requiredFeatureCount = adapterFeatures.size();
    //deviceDesc.requiredFeatures = adapterFeatures.data();
    //deviceDesc.requiredLimits = &requiredLimits;
    //deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "DefaultQueue";
    deviceDesc.deviceLostCallback = DeviceLostCallback;
    deviceDesc.deviceLostUserdata = this;
    deviceDesc.uncapturedErrorCallbackInfo.callback = UncapturedErrorCallback;
    deviceDesc.uncapturedErrorCallbackInfo.userdata = this;

    wgpuAdapterRequestDevice(
        handle,
        &deviceDesc,
        onDeviceRequestEnded,
        &userData
    );

#ifdef __EMSCRIPTEN__
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__

    ALIMER_ASSERT(userData.requestEnded);

    WebGPUDevice* device = new WebGPUDevice();
    device->handle = userData.device;
    return device;
}

/* WebGPUInstance */
WebGPUInstance::~WebGPUInstance()
{
    if (handle)
    {
        wgpuInstanceRelease(handle);
        handle = nullptr;
    }
}

GPUSurface WebGPUInstance::CreateSurface(Window* window)
{
    ALIMER_UNUSED(window);
    return nullptr;
}

GPUAdapter WebGPUInstance::RequestAdapter(const GPURequestAdapterOptions* options)
{
    // A simple structure holding the local information shared with the onAdapterRequestEnded callback.
    struct UserData {
        WGPUAdapter adapter = nullptr;
        bool requestEnded = false;
    } userData;

    // Callback called by wgpuInstanceRequestAdapter when the request returns
    // This is a C++ lambda function, but could be any function defined in the
    // global scope. It must be non-capturing (the brackets [] are empty) so
    // that it behaves like a regular C function pointer, which is what
    // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
    // is to convey what we want to capture through the pUserData pointer,
    // provided as the last argument of wgpuInstanceRequestAdapter and received
    // by the callback as its last argument.
    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* pUserData) {
        UserData& userData = *reinterpret_cast<UserData*>(pUserData);
        if (status == WGPURequestAdapterStatus_Success) {
            userData.adapter = adapter;
        }
        else {
            alimerLogError(LogCategory_GPU, "Could not get WebGPU adapter: {}", message);
        }
        userData.requestEnded = true;
        };

    WGPURequestAdapterOptions gpuOptions = {};
    gpuOptions.nextInChain = nullptr;
    gpuOptions.powerPreference = options ? ToWGPU(options->powerPreference) : WGPUPowerPreference_Undefined;
#if ALIMER_PLATFORM_WINDOWS
    gpuOptions.backendType = WGPUBackendType_D3D12;
#elif ALIMER_PLATFORM_LINUX
    gpuOptions.backendType = WGPUBackendType_Vulkan;
#endif

    // Call to the WebGPU request adapter procedure
    wgpuInstanceRequestAdapter(
        handle /* equivalent of navigator.gpu */,
        &gpuOptions,
        onAdapterRequestEnded,
        (void*)&userData
    );

    // We wait until userData.requestEnded gets true
#if defined(__EMSCRIPTEN__)
    while (!userData.requestEnded) {
        emscripten_sleep(100);
    }
#endif // __EMSCRIPTEN__


    // We wait until userData.requestEnded gets true
    // [...] Wait for request to end

    ALIMER_ASSERT(userData.requestEnded);

    WebGPUAdapter* result = new WebGPUAdapter();
    result->handle = userData.adapter;
    return result;
}

bool WGPU_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;

#if !defined(__EMSCRIPTEN__)

#if defined(_WIN32)
    wgpu_state.wgpu_module = LoadLibraryW(L"wgpu_native.dll");
    if (!wgpu_state.wgpu_module)
    {
        wgpu_state.wgpu_module = LoadLibraryW(L"dawn.dll");
        if (wgpu_state.wgpu_module)
            wgpu_state.dawn = true;
    }

    if (!wgpu_state.wgpu_module)
        return false;
#elif defined(__APPLE__)
    wgpu_state.wgpu_module = dlopen("libwgpu_native.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!wgpu_state.wgpu_module)
    {
        wgpu_state.wgpu_module = dlopen("libdawn.dylib", RTLD_NOW | RTLD_LOCAL);
        if (wgpu_state.wgpu_module)
            wgpu_state.dawn = true;
    }

    if (!wgpu_state.wgpu_module)
        return false;
#else
    wgpu_state.wgpu_module = dlopen("libwgpu_native.so", RTLD_NOW | RTLD_LOCAL);
    if (!wgpu_state.wgpu_module)
    {
        wgpu_state.wgpu_module = dlopen("libdawn.so", RTLD_NOW | RTLD_LOCAL);
        if (wgpu_state.wgpu_module)
            wgpu_state.dawn = true;
    }

    if (!wgpu_state.wgpu_module)
        return false;
#endif

#if defined(_WIN32)
#define LOAD_PROC(name) wgpu##name = (WGPUProc##name)GetProcAddress(wgpu_state.wgpu_module, "wgpu" #name);
#else
#define LOAD_PROC(name) wgpu##name = (WGPUProc##name)dlsym(wgpu_state.wgpu_module, "wgpu" #name);
#endif

    ALIMER_RHI_WGPU_PROCS(LOAD_PROC);
#if defined(WEBGPU_BACKEND_WGPU)
    ALIMER_RHI_WGPU_NATIVE_PROCS(LOAD_PROC);
#endif
#undef LOAD_PROC
#endif /* !defined(__EMSCRIPTEN__) */

    available = true;
    return true;
}

GPUInstance* WGPU_CreateInstance(const GPUConfig* config)
{
    WGPUInstance instance = nullptr;
#if defined(__EMSCRIPTEN__)
    instance = wgpuCreateInstance(nullptr);
#else
    // We create the instance using this descriptor
    WGPUInstanceDescriptor instanceDesc = {};
    instanceDesc.nextInChain = nullptr;
    if (!wgpu_state.dawn)
    {
        WGPUInstanceExtras instanceExtras = {};
        if (config->validationMode != GPUValidationMode_Disabled)
        {
            instanceExtras.flags = WGPUInstanceFlag_Validation;
        };
        instanceDesc.nextInChain = (WGPUChainedStruct*)&instanceExtras;
    }

    instance = wgpuCreateInstance(&instanceDesc);
#endif
    if (!instance)
        return nullptr;

    alimerLogInfo(LogCategory_GPU, "WebGPU: Initialized with success");
    WebGPUInstance* result = new WebGPUInstance();
    result->handle = instance;
    return result;
}

#endif /* defined(ALIMER_GPU_WGPU) */
