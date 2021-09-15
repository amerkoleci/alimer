// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Graphics.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12Pipeline.h"
#include "D3D12SwapChain.h"
#include "D3D12Shader.h"
#include "D3D12Sampler.h"
#include "D3D12CommandBuffer.h"
#include "directx/d3dx12.h"

namespace Alimer
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

        static DxcCreateInstanceProc DxcCreateInstance;
#else
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#endif

#ifdef _DEBUG
        // Declare debug guids to avoid linking with "dxguid.lib"
        static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
        static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };

        static std::wstring SizeToStr(size_t size)
        {
            if (size == 0)
                return L"0";
            wchar_t result[32];
            double size2 = (double)size;
            if (size2 >= 1024.0 * 1024.0 * 1024.0 * 1024.0)
            {
                swprintf_s(result, L"%.2f TB", size2 / (1024.0 * 1024.0 * 1024.0 * 1024.0));
            }
            else if (size2 >= 1024.0 * 1024.0 * 1024.0)
            {
                swprintf_s(result, L"%.2f GB", size2 / (1024.0 * 1024.0 * 1024.0));
            }
            else if (size2 >= 1024.0 * 1024.0)
            {
                swprintf_s(result, L"%.2f MB", size2 / (1024.0 * 1024.0));
            }
            else if (size2 >= 1024.0)
            {
                swprintf_s(result, L"%.2f KB", size2 / 1024.0);
            }
            else
                swprintf_s(result, L"%llu B", size);
            return result;
        }
#endif
    }

    bool D3D12Graphics::IsAvailable()
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
        HMODULE dxcompiler = LoadLibraryW(L"dxcompiler.dll");

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

        if (dxcompiler)
        {
            DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(dxcompiler, "DxcCreateInstance");
            ALIMER_ASSERT(DxcCreateInstance != nullptr);
        }

        if (SUCCEEDED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
        {
            available = true;
            return true;
        }

        return false;
    }

    D3D12Graphics::D3D12Graphics(ValidationMode validationMode)
    {
        ALIMER_VERIFY(IsAvailable());

        if (DxcCreateInstance != nullptr)
        {
            ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
        }

        if (validationMode != ValidationMode::Disabled)
        {
            ComPtr<ID3D12Debug> d3d12Debug;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(d3d12Debug.GetAddressOf()))))
            {
                d3d12Debug->EnableDebugLayer();

                if (validationMode == ValidationMode::GPU)
                {
                    ComPtr<ID3D12Debug1> d3d12Debug1;
                    if (SUCCEEDED(d3d12Debug.As(&d3d12Debug1)))
                    {
                        d3d12Debug1->SetEnableGPUBasedValidation(TRUE);
                        d3d12Debug1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                    }

                    ComPtr<ID3D12Debug2> d3d12Debug2;
                    if (SUCCEEDED(d3d12Debug.As(&d3d12Debug2)))
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

        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

        // Determines whether tearing support is available for fullscreen borderless windows.
        {
            BOOL allowTearing = FALSE;

            IDXGIFactory5* dxgiFactory5 = nullptr;
            HRESULT hr = dxgiFactory->QueryInterface(&dxgiFactory5);
            if (SUCCEEDED(hr))
            {
                hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
            }

            if (FAILED(hr) || !allowTearing)
            {
                tearingSupported = false;
#ifdef _DEBUG
                OutputDebugStringA("WARNING: Variable refresh rate displays not supported");
#endif
            }
            else
            {
                tearingSupported = true;
            }
            SafeRelease(dxgiFactory5);
        }

        {
            // Feature levels to try creating devices. Listed in descending order so the highest supported level is used.
            const static D3D_FEATURE_LEVEL kFeatureLevels[] =
            {
                D3D_FEATURE_LEVEL_12_2,
                D3D_FEATURE_LEVEL_12_1,
                D3D_FEATURE_LEVEL_12_0,
            };

            //ComPtr<IDXGIFactory6> dxgiFactory6;
            //const bool enumByPreference = SUCCEEDED(dxgiFactory.As(&dxgiFactory6));

            ComPtr<IDXGIAdapter1> adapter;
            GetAdapter(adapter.GetAddressOf());

            // Create the DX12 API device object.
            HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
            ThrowIfFailed(hr);
            device->SetName(L"AlimerDevice");

            // Configure debug device (if active).
            ID3D12InfoQueue* d3d12InfoQueue;
            if (SUCCEEDED(device->QueryInterface(&d3d12InfoQueue)))
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
                d3d12InfoQueue->Release();
            }

            // Init capabilities.
            DXGI_ADAPTER_DESC1 adapterDesc;
            ThrowIfFailed(adapter->GetDesc1(&adapterDesc));

            // Init feature check
            CD3DX12FeatureSupport d3dFeatures;
            ThrowIfFailed(d3dFeatures.Init(device));
            featureLevel = d3dFeatures.MaxSupportedFeatureLevel();

#if TODO
            caps.backendType = GPUBackendType::Direct3D12;
            caps.vendorId = adapterDesc.VendorId;
            caps.adapterId = adapterDesc.DeviceId;

            // Detect adapter type.
            if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                caps.adapterType = GPUAdapterType::Software;
            }
            else
            {
                D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
                ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)));
                caps.adapterType = arch.UMA ? GPUAdapterType::Integrated : GPUAdapterType::Discrete;
            }

            // Determine maximum supported feature level for this device
            D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
            {
                _countof(kFeatureLevels), kFeatureLevels, D3D_FEATURE_LEVEL_11_0
            };

            hr = device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
            if (SUCCEEDED(hr))
            {
                featureLevel = featLevels.MaxSupportedFeatureLevel;
            }
            else
            {
                featureLevel = D3D_FEATURE_LEVEL_11_0;
            }
#endif // TODO


            LOGI("Create Direct3D12 device {} with adapter: VID:{:#04x}, PID:{:#04x} - {}",
                ToString(featureLevel),
                adapterDesc.VendorId,
                adapterDesc.DeviceId,
                ToUtf8(adapterDesc.Description)
            );

            D3D12_FEATURE_DATA_ROOT_SIGNATURE dataRootSignature = {};
            dataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            ThrowIfFailed(
                device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &dataRootSignature, sizeof(dataRootSignature))
            );
            if (dataRootSignature.HighestVersion < D3D_ROOT_SIGNATURE_VERSION_1_1)
            {
                //ErrorDialog("Error", "Direct3D12: Root signature version 1.1 not supported!");
                return;
            }

#if TODO
            caps.blobType = ShaderBlobType::DXIL;
            caps.features.independentBlend = true;
            caps.features.computeShader = true;
            caps.features.multiViewport = true;
            caps.features.indexUInt32 = true;
            caps.features.fillModeNonSolid = true;
            caps.features.samplerAnisotropy = true;
            caps.features.textureCompressionETC2 = false;
            caps.features.textureCompressionASTC_LDR = false;
            caps.features.textureCompressionBC = true;
            caps.features.textureCubeArray = (featureLevel >= D3D_FEATURE_LEVEL_10_1);
            caps.features.bindlessDescriptors = true;
            caps.features.raytracing = false;

            D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5{};
            D3D12_FEATURE_DATA_D3D12_OPTIONS6 features6{};
            D3D12_FEATURE_DATA_D3D12_OPTIONS7 features7{};

            ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(features5)));
            ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &features6, sizeof(features6)));
            ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features7, sizeof(features7)));

            if (features5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0)
            {
                caps.features.raytracing = true;
                if (features5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1)
                {
                    caps.features.raytracingInline = true;
                }
            }

            supportsRenderPass = features5.RenderPassesTier > D3D12_RENDER_PASS_TIER_0 && caps.vendorId != KnownVendorId_Intel;

            if (features6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1)
            {
                caps.features.variableRateShading = true;

                if (features6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2)
                {
                    caps.features.variableRateShadingExtended = true;
                    //VARIABLE_RATE_SHADING_TILE_SIZE = features6.ShadingRateImageTileSize;
                }
            }

            if (features7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1)
            {
                caps.features.meshShader = true;
            }

            // Limits
            caps.limits.maxVertexAttributes = kMaxVertexAttributes;
            caps.limits.maxVertexBindings = kMaxVertexBufferBindings;
            caps.limits.maxVertexAttributeOffset = kMaxVertexAttributeOffset;
            caps.limits.maxVertexBindingStride = kMaxVertexBufferStride;

            caps.limits.maxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            caps.limits.maxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            caps.limits.maxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
            caps.limits.maxTextureArrayLayers = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            caps.limits.maxColorAttachments = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
            caps.limits.maxUniformBufferSize = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
            caps.limits.maxStorageBufferSize = UINT32_MAX;
            caps.limits.minUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
            caps.limits.minStorageBufferOffsetAlignment = 16;
            caps.limits.maxSamplerAnisotropy = D3D12_MAX_MAXANISOTROPY;
            caps.limits.maxViewports = D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
            caps.limits.maxViewportWidth = D3D12_VIEWPORT_BOUNDS_MAX;
            caps.limits.maxViewportHeight = D3D12_VIEWPORT_BOUNDS_MAX;
            caps.limits.maxTessellationPatchSize = D3D12_IA_PATCH_MAX_CONTROL_POINT_COUNT;
            //caps.limits.pointSizeRangeMin = 1.0f;
            //caps.limits.pointSizeRangeMax = 1.0f;
            //caps.limits.lineWidthRangeMin = 1.0f;
            //caps.limits.lineWidthRangeMax = 1.0f;
            caps.limits.maxComputeSharedMemorySize = D3D12_CS_THREAD_LOCAL_TEMP_REGISTER_POOL;
            caps.limits.maxComputeWorkGroupCountX = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupCountY = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupCountZ = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION;
            caps.limits.maxComputeWorkGroupInvocations = D3D12_CS_THREAD_GROUP_MAX_THREADS_PER_GROUP;
            caps.limits.maxComputeWorkGroupSizeX = D3D12_CS_THREAD_GROUP_MAX_X;
            caps.limits.maxComputeWorkGroupSizeY = D3D12_CS_THREAD_GROUP_MAX_Y;
            caps.limits.maxComputeWorkGroupSizeZ = D3D12_CS_THREAD_GROUP_MAX_Z;

            /* see: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_format_support */
            UINT formatSupport = 0;
            for (int fmt = static_cast<uint32_t>(PixelFormat::Undefined) + 1; fmt < static_cast<uint32_t>(PixelFormat::Count); fmt++)
            {
                D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport = { ToDXGIFormat((PixelFormat)fmt), D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE };

                if (formatSupport.Format == DXGI_FORMAT_UNKNOWN)
                    continue;

                HRESULT hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(formatSupport));
                if (FAILED(hr))
                    continue;

                PixelFormatFeatures features = PixelFormatFeatures::None;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)
                    features |= PixelFormatFeatures::Sampled;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET)
                    features |= PixelFormatFeatures::RenderTarget;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)
                    features |= PixelFormatFeatures::DepthStencil;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_BLENDABLE)
                    features |= PixelFormatFeatures::RenderTargetBlend;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) // Is this correct?
                    features |= PixelFormatFeatures::Filter;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_LOAD)
                    features |= PixelFormatFeatures::Storage;

                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RENDERTARGET)
                    features |= PixelFormatFeatures::Blit;
                if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_MULTISAMPLE_RESOLVE)
                    features |= PixelFormatFeatures::Blit;

                caps.formatProperties[fmt].features = features;
            }
#endif // TODO


            D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
            allocatorDesc.pDevice = device;
            allocatorDesc.pAdapter = adapter.Get();

            if (FAILED(D3D12MA::CreateAllocator(&allocatorDesc, &allocator)))
            {
                return;
            }

#ifdef _DEBUG
            PrintAdapterInformation(adapter.Get());
#endif
        }

        {
            // Create the command queues
            graphicsQueue = new D3D12CommandQueue(*this, CommandQueueType::Graphics);
            computeQueue = new D3D12CommandQueue(*this, CommandQueueType::Compute);
        }

        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFence)));
        frameFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

        // Descriptor allocators
        {
            rtv.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1024);
            dsv.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256);
            resourceDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024);
            samplerDescriptorAllocator.Init(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024);

        }

        // Shader visible descriptor resource heap and bindless.
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.NumDescriptors = 1000000; // tier 1 limit
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&resourceHeap.handle)));
            resourceHeap.CPUStart = resourceHeap.handle->GetCPUDescriptorHandleForHeapStart();
            resourceHeap.GPUStart = resourceHeap.handle->GetGPUDescriptorHandleForHeapStart();

            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&resourceHeap.fence)));
            resourceHeap.fenceValue = resourceHeap.fence->GetCompletedValue();

            for (uint32_t i = 0; i < kBindlessResourceCapacity; ++i)
            {
                freeBindlessResources.push_back(kBindlessResourceCapacity - i - 1);
            }
        }

        // Shader visible descriptor sampler heap and bindless.
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
            heapDesc.NumDescriptors = 2048; // tier 1 limit
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&samplerHeap.handle)));
            samplerHeap.CPUStart = samplerHeap.handle->GetCPUDescriptorHandleForHeapStart();
            samplerHeap.GPUStart = samplerHeap.handle->GetGPUDescriptorHandleForHeapStart();

            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&samplerHeap.fence)));
            samplerHeap.fenceValue = samplerHeap.fence->GetCompletedValue();

            for (uint32_t i = 0; i < kBindlessSamplerCapacity; ++i)
            {
                freeBindlessSamplers.push_back(kBindlessSamplerCapacity - i - 1);
            }
        }

        // Create upload data
        {
            for (uint64_t i = 0; i < kMaxUploadSubmissions; ++i)
            {
                UploadSubmission& submission = uploadSubmissions[i];
                ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&submission.commandAllocator)));
                ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, submission.commandAllocator, nullptr, IID_PPV_ARGS(&submission.commandList)));
                ThrowIfFailed(submission.commandList->Close());

                submission.commandList->SetName(L"Upload Command List");
            }

            D3D12_COMMAND_QUEUE_DESC copyQueueDesc = {};
            copyQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            copyQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            copyQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            copyQueueDesc.NodeMask = 0;
            ThrowIfFailed(device->CreateCommandQueue(&copyQueueDesc, IID_PPV_ARGS(&uploadCmdQueue)));
            uploadCmdQueue->SetName(L"Upload Copy Queue");

            ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence)));
            uploadFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

            D3D12_RESOURCE_DESC resourceDesc = { };
            resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resourceDesc.Width = kUploadBufferSize;
            resourceDesc.Height = 1;
            resourceDesc.DepthOrArraySize = 1;
            resourceDesc.MipLevels = 1;
            resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
            resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            resourceDesc.SampleDesc.Count = 1;
            resourceDesc.SampleDesc.Quality = 0;
            resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resourceDesc.Alignment = 0;

            D3D12MA::ALLOCATION_DESC allocDesc{};
            allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

            ThrowIfFailed(allocator->CreateResource(
                &allocDesc,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                &uploadBufferAllocation,
                IID_PPV_ARGS(&uploadBuffer)
            ));

            D3D12_RANGE readRange = { };
            ThrowIfFailed(uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&uploadBufferCPUAddr)));
        }

        LOGI("Direct3D12 graphics backend initialized with success");
    }

    D3D12Graphics::~D3D12Graphics()
    {
        shuttingDown = true;

        {
            SafeRelease(frameFence);
            CloseHandle(frameFenceEvent);
        }

        // CPU Descriptor Heaps
        rtv.Shutdown();
        dsv.Shutdown();
        resourceDescriptorAllocator.Shutdown();
        samplerDescriptorAllocator.Shutdown();

        // GPU Descriptor Heaps
        SafeRelease(resourceHeap.handle);
        SafeRelease(resourceHeap.fence);
        SafeRelease(samplerHeap.handle);
        SafeRelease(samplerHeap.fence);

        delete graphicsQueue; graphicsQueue = nullptr;
        delete computeQueue; computeQueue = nullptr;

        // Release upload data
        {
            SafeRelease(uploadBufferAllocation);
            SafeRelease(uploadBuffer);
            SafeRelease(uploadCmdQueue);

            CloseHandle(uploadFenceEvent);
            SafeRelease(uploadFence);

            for (uint64_t i = 0; i < kMaxUploadSubmissions; ++i)
            {
                SafeRelease(uploadSubmissions[i].commandAllocator);
                SafeRelease(uploadSubmissions[i].commandList);
            }
        }

        // Destroy pending resources that still exist.
        Destroy();

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

            SafeRelease(allocator);
        }

        ULONG refCount = device->Release();
#if !defined(NDEBUG)
        if (refCount > 0)
        {
            LOGD("Direct3D12: There are {} unreleased references left on the device", refCount);

            ID3D12DebugDevice* debugDevice = nullptr;
            if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D12DebugDevice), (void**)&debugDevice)))
            {
                debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
                debugDevice->Release();
            }
        }
#else
        (void)refCount;
#endif
        device = nullptr;

        dxgiFactory.Reset();

#ifdef _DEBUG
        {
            IDXGIDebug1* dxgiDebug1;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug1))))
            {
                dxgiDebug1->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
                dxgiDebug1->Release();
            }
        }
#endif
    }

    void D3D12Graphics::GetAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        ComPtr<IDXGIFactory6> factory6;
        HRESULT hr = dxgiFactory.As(&factory6);
        if (SUCCEEDED(hr))
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
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
                ++adapterIndex)
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

    D3D12DescriptorAlloc D3D12Graphics::AllocateSRV()
    {
        AcquireSRWLockExclusive(&bindlessFreeLock);
        if (!freeBindlessResources.empty())
        {
            uint32_t index = freeBindlessResources.back();
            freeBindlessResources.pop_back();
            ReleaseSRWLockExclusive(&bindlessFreeLock);

            D3D12DescriptorAlloc alloc;
            alloc.handle = resourceHeap.CPUStart;
            alloc.handle.ptr += index * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            alloc.index = index;
            return alloc;
        }

        ReleaseSRWLockExclusive(&bindlessFreeLock);
        return {};
    }

    void D3D12Graphics::HandleDeviceLost(HRESULT hr)
    {
#ifdef _DEBUG
        char buff[64] = {};
        sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
            static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? device->GetDeviceRemovedReason() : hr));
        OutputDebugStringA(buff);
#endif

        deviceLost = true;
    }

#ifdef _DEBUG
    void D3D12Graphics::PrintAdapterInformation(IDXGIAdapter1* adapter)
    {
        DXGI_ADAPTER_DESC1 adapterDesc = {};
        adapter->GetDesc1(&adapterDesc);

        wprintf(L"DXGI_ADAPTER_DESC1:\n");
        wprintf(L"    Description = %s\n", adapterDesc.Description);
        printf("    VendorId = 0x%X (%s)\n", adapterDesc.VendorId, GetVendorName(adapterDesc.VendorId));
        wprintf(L"    DeviceId = 0x%X\n", adapterDesc.DeviceId);
        wprintf(L"    SubSysId = 0x%X\n", adapterDesc.SubSysId);
        wprintf(L"    Revision = 0x%X\n", adapterDesc.Revision);
        wprintf(L"    DedicatedVideoMemory = %zu B (%s)\n", adapterDesc.DedicatedVideoMemory, SizeToStr(adapterDesc.DedicatedVideoMemory).c_str());
        wprintf(L"    DedicatedSystemMemory = %zu B (%s)\n", adapterDesc.DedicatedSystemMemory, SizeToStr(adapterDesc.DedicatedSystemMemory).c_str());
        wprintf(L"    SharedSystemMemory = %zu B (%s)\n", adapterDesc.SharedSystemMemory, SizeToStr(adapterDesc.SharedSystemMemory).c_str());

        const D3D12_FEATURE_DATA_D3D12_OPTIONS& options = allocator->GetD3D12Options();
        wprintf(L"D3D12_FEATURE_DATA_D3D12_OPTIONS:\n");
        wprintf(L"    StandardSwizzle64KBSupported = %u\n", options.StandardSwizzle64KBSupported ? 1 : 0);
        wprintf(L"    CrossAdapterRowMajorTextureSupported = %u\n", options.CrossAdapterRowMajorTextureSupported ? 1 : 0);
        switch (options.ResourceHeapTier)
        {
        case D3D12_RESOURCE_HEAP_TIER_1:
            wprintf(L"    ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_1\n");
            break;
        case D3D12_RESOURCE_HEAP_TIER_2:
            wprintf(L"    ResourceHeapTier = D3D12_RESOURCE_HEAP_TIER_2\n");
            break;
        default:
            ALIMER_UNREACHABLE();
        }

        IDXGIAdapter3* adapter3;
        if (SUCCEEDED(adapter->QueryInterface(&adapter3)))
        {
            wprintf(L"DXGI_QUERY_VIDEO_MEMORY_INFO:\n");
            for (UINT groupIndex = 0; groupIndex < 2; ++groupIndex)
            {
                const DXGI_MEMORY_SEGMENT_GROUP group = groupIndex == 0 ? DXGI_MEMORY_SEGMENT_GROUP_LOCAL : DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL;
                const wchar_t* const groupName = groupIndex == 0 ? L"DXGI_MEMORY_SEGMENT_GROUP_LOCAL" : L"DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL";
                DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
                ThrowIfFailed(adapter3->QueryVideoMemoryInfo(0, group, &info));
                wprintf(L"    %s:\n", groupName);
                wprintf(L"        Budget = %llu B (%s)\n", info.Budget, SizeToStr(info.Budget).c_str());
                wprintf(L"        CurrentUsage = %llu B (%s)\n", info.CurrentUsage, SizeToStr(info.CurrentUsage).c_str());
                wprintf(L"        AvailableForReservation = %llu B (%s)\n", info.AvailableForReservation, SizeToStr(info.AvailableForReservation).c_str());
                wprintf(L"        CurrentReservation = %llu B (%s)\n", info.CurrentReservation, SizeToStr(info.CurrentReservation).c_str());
            }
            adapter3->Release();
        }

        ALIMER_ASSERT(device);
        D3D12_FEATURE_DATA_ARCHITECTURE1 architecture1 = {};
        if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, &architecture1, sizeof architecture1)))
        {
            wprintf(L"D3D12_FEATURE_DATA_ARCHITECTURE1:\n");
            wprintf(L"    UMA: %u\n", architecture1.UMA ? 1 : 0);
            wprintf(L"    CacheCoherentUMA: %u\n", architecture1.CacheCoherentUMA ? 1 : 0);
            wprintf(L"    IsolatedMMU: %u\n", architecture1.IsolatedMMU ? 1 : 0);
        }
    }
#endif

    void D3D12Graphics::WaitIdle()
    {
        ToD3D12(computeQueue)->WaitIdle();
        ToD3D12(graphicsQueue)->WaitIdle();

        ToD3D12Handle(graphicsQueue)->Signal(frameFence, ++frameCount);
        frameFence->SetEventOnCompletion(frameCount, frameFenceEvent);
        WaitForSingleObject(frameFenceEvent, INFINITE);

        frameIndex = frameCount % kMaxFramesInFlight;
        ProcessDeletionQueue();
    }

    void D3D12Graphics::FinishFrame()
    {
        ToD3D12(graphicsQueue)->GetHandle()->Signal(frameFence, ++frameCount);

        uint64_t GPUFrameCount = frameFence->GetCompletedValue();

        if ((frameCount - GPUFrameCount) >= kMaxFramesInFlight)
        {
            frameFence->SetEventOnCompletion(GPUFrameCount + 1, frameFenceEvent);
            WaitForSingleObject(frameFenceEvent, INFINITE);
        }

        frameIndex = frameCount % kMaxFramesInFlight;
        ProcessDeletionQueue();

        if (!dxgiFactory->IsCurrent())
        {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return resourceDescriptorAllocator.Allocate();
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return samplerDescriptorAllocator.Allocate();
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return rtv.Allocate();
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return dsv.Allocate();
        default:
            ALIMER_UNREACHABLE();
            return {};
        }
    }

    void D3D12Graphics::FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_CPU_DESCRIPTOR_HANDLE handle)
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            resourceDescriptorAllocator.Free(handle);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            samplerDescriptorAllocator.Free(handle);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            rtv.Free(handle);
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            dsv.Free(handle);
            break;
        default:
            ALIMER_UNREACHABLE();
            break;
        }
    }

    uint32_t D3D12Graphics::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return resourceDescriptorAllocator.descriptorSize;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return samplerDescriptorAllocator.descriptorSize;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            return rtv.descriptorSize;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            return dsv.descriptorSize;
        default:
            ALIMER_UNREACHABLE();
        }
    }

    void D3D12Graphics::SyncCopyQueue()
    {
        // If we can grab the lock, try to clear out any completed submissions
        if (TryAcquireSRWLockExclusive(&uploadSubmissionLock))
        {
            ClearFinishedUploads(0);

            ReleaseSRWLockExclusive(&uploadSubmissionLock);
        }

        {
            AcquireSRWLockExclusive(&uploadQueueLock);

            // Make sure to sync on any pending uploads
            ClearFinishedUploads(0);
            ToD3D12Handle(graphicsQueue)->Wait(uploadFence, uploadFenceValue);

            ReleaseSRWLockExclusive(&uploadQueueLock);
        }

        //TempFrameUsed = 0;
    }

    void D3D12Graphics::DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation)
    {
        std::lock_guard<std::mutex> guard(destroyMutex);

        if (shuttingDown)
        {
            resource->Release();
            SafeRelease(allocation);
            return;
        }

        deferredReleases.push_back(std::make_pair(resource, frameCount));
        if (allocation != nullptr)
        {
            deferredAllocations.push_back(std::make_pair(allocation, frameCount));
        }
    }

    void D3D12Graphics::ProcessDeletionQueue()
    {
        std::lock_guard<std::mutex> guard(destroyMutex);

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
    }

    ID3D12RootSignature* D3D12Graphics::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
    {
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRSDesc = {};
        versionedRSDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versionedRSDesc.Desc_1_1 = desc;

        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedRSDesc, blob.GetAddressOf(), errorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            LOGE("Failed to create root signature: {}", (char*)errorBlob->GetBufferPointer());
            return nullptr;
        }

        ID3D12RootSignature* result;
        ThrowIfFailed(
            device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&result))
        );

        return result;
    }

    void D3D12Graphics::ClearFinishedUploads(uint64_t flushCount)
    {
        const uint64_t start = uploadSubmissionStart;
        const uint64_t used = uploadSubmissionUsed;
        for (uint64_t i = 0; i < used; ++i)
        {
            const uint64_t idx = (start + i) % kMaxUploadSubmissions;
            UploadSubmission& submission = uploadSubmissions[idx];
            ALIMER_ASSERT(submission.size > 0);
            ALIMER_ASSERT(uploadBufferUsed >= submission.size);

            // If the submission hasn't been sent to the GPU yet we can't wait for it
            if (submission.fenceValue == uint64_t(-1))
                break;

            if (i < flushCount)
            {
                if (uploadFence->GetCompletedValue() < submission.fenceValue)
                {
                    ThrowIfFailed(uploadFence->SetEventOnCompletion(submission.fenceValue, uploadFenceEvent));
                    WaitForSingleObject(uploadFenceEvent, INFINITE);
                }
            }

            // Check if signalled
            if (uploadFence->GetCompletedValue() >= submission.fenceValue)
            {
                uploadSubmissionStart = (uploadSubmissionStart + 1) % kMaxUploadSubmissions;
                uploadSubmissionUsed -= 1;
                uploadBufferStart = (uploadBufferStart + submission.padding) % kUploadBufferSize;
                ALIMER_ASSERT(submission.offset == uploadBufferStart);
                ALIMER_ASSERT(uploadBufferStart + submission.size <= kUploadBufferSize);
                uploadBufferStart = (uploadBufferStart + submission.size) % kUploadBufferSize;
                uploadBufferUsed -= (submission.size + submission.padding);
                submission.Reset();

                if (uploadBufferUsed == 0)
                    uploadBufferStart = 0;
            }
            else
            {
                // We don't want to retire our submissions out of allocation order, because
                // the ring buffer logic above will move the tail position forward (we don't
                // allow holes in the ring buffer). Submitting out-of-order should still be
                // ok though as long as we retire in-order.
                break;
            }
        }
    }

    D3D12Graphics::UploadSubmission* D3D12Graphics::AllocUploadSubmission(uint64_t size)
    {
        ALIMER_ASSERT(uploadSubmissionUsed <= kMaxUploadSubmissions);
        if (uploadSubmissionUsed == kMaxUploadSubmissions)
            return nullptr;

        const uint64_t submissionIdx = (uploadSubmissionStart + uploadSubmissionUsed) % kMaxUploadSubmissions;
        ALIMER_ASSERT(uploadSubmissions[submissionIdx].size == 0);

        ALIMER_ASSERT(uploadBufferUsed <= kUploadBufferSize);
        if (size > (kUploadBufferSize - uploadBufferUsed))
            return nullptr;

        const uint64_t start = uploadBufferStart;
        const uint64_t end = uploadBufferStart + uploadBufferUsed;
        uint64_t allocOffset = uint64_t(-1);
        uint64_t padding = 0;
        if (end < kUploadBufferSize)
        {
            const uint64_t endAmt = kUploadBufferSize - end;
            if (endAmt >= size)
            {
                allocOffset = end;
            }
            else if (start >= size)
            {
                // Wrap around to the beginning
                allocOffset = 0;
                uploadBufferUsed += endAmt;
                padding = endAmt;
            }
        }
        else
        {
            const uint64_t wrappedEnd = end % kUploadBufferSize;
            if ((start - wrappedEnd) >= size)
                allocOffset = wrappedEnd;
        }

        if (allocOffset == uint64_t(-1))
            return nullptr;

        uploadSubmissionUsed += 1;
        uploadBufferUsed += size;

        UploadSubmission* submission = &uploadSubmissions[submissionIdx];
        submission->offset = allocOffset;
        submission->size = size;
        submission->fenceValue = uint64_t(-1);
        submission->padding = padding;

        return submission;
    }

    D3D12UploadContext D3D12Graphics::ResourceUploadBegin(uint64_t size)
    {
        size = AlignUp(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        ALIMER_ASSERT(size <= kUploadBufferSize);
        ALIMER_ASSERT(size > 0);

        UploadSubmission* submission = nullptr;

        {
            AcquireSRWLockExclusive(&uploadSubmissionLock);

            ClearFinishedUploads(0);

            submission = AllocUploadSubmission(size);
            while (submission == nullptr)
            {
                ClearFinishedUploads(1);
                submission = AllocUploadSubmission(size);
            }

            ReleaseSRWLockExclusive(&uploadSubmissionLock);
        }

        ThrowIfFailed(submission->commandAllocator->Reset());
        ThrowIfFailed(submission->commandList->Reset(submission->commandAllocator, nullptr));

        D3D12UploadContext context;
        context.commandList = submission->commandList;
        context.resource = uploadBuffer;
        context.CPUAddress = uploadBufferCPUAddr + submission->offset;
        context.resourceOffset = submission->offset;
        context.submission = submission;

        return context;
    }

    void D3D12Graphics::ResourceUploadEnd(D3D12UploadContext& context)
    {
        ALIMER_ASSERT(context.commandList != nullptr);
        ALIMER_ASSERT(context.submission != nullptr);
        UploadSubmission* submission = reinterpret_cast<UploadSubmission*>(context.submission);

        {
            AcquireSRWLockExclusive(&uploadQueueLock);

            // Finish off and execute the command list
            ThrowIfFailed(submission->commandList->Close());
            ID3D12CommandList* cmdLists[1] = {
                submission->commandList
            };
            uploadCmdQueue->ExecuteCommandLists(1, cmdLists);

            ++uploadFenceValue;
            ThrowIfFailed(uploadCmdQueue->Signal(uploadFence, uploadFenceValue));
            submission->fenceValue = uploadFenceValue;

            ReleaseSRWLockExclusive(&uploadQueueLock);
        }

        context = D3D12UploadContext();
    }

    TextureRef D3D12Graphics::CreateTextureCore(const TextureCreateInfo& info, const void* initialData)
    {
        auto result = new D3D12Texture(*this, info, initialData);

        if (result->GetHandle() != nullptr)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    BufferRef D3D12Graphics::CreateBuffer(const BufferCreateInfo& info, const void* initialData)
    {
        auto result = new D3D12Buffer(*this, info, initialData);

        if (result->GetHandle() != nullptr)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    ShaderRef D3D12Graphics::CreateShader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint)
    {
        auto result = new D3D12Shader(*this, stage, byteCode, entryPoint);
        return ShaderRef::Create(result);
    }

    SamplerRef D3D12Graphics::CreateSampler(const SamplerCreateInfo* info)
    {
        auto result = new D3D12Sampler(*this, info);
        return SamplerRef::Create(result);
    }

    PipelineRef D3D12Graphics::CreateRenderPipeline(const RenderPipelineStateCreateInfo* info)
    {
        auto result = new D3D12Pipeline(*this, info);
        if (result->GetHandle() != nullptr)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    PipelineRef D3D12Graphics::CreateComputePipeline(const ComputePipelineCreateInfo* info)
    {
        auto result = new D3D12Pipeline(*this, info);
        if (result->GetHandle() != nullptr)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    SwapChainRef D3D12Graphics::CreateSwapChain(void* windowHandle, const SwapChainCreateInfo& info)
    {
        auto result = new D3D12SwapChain(*this, windowHandle, info);

        if (result->GetHandle() != nullptr)
        {
            return SwapChainRef::Create(result);
        }

        delete result;
        return nullptr;
    }
}
