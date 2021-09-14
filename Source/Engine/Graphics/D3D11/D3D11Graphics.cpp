// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D11Graphics.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "Core/StringUtils.h"
#include "Core/Log.h"
#include "Window.h"

#ifdef _DEBUG
#   include <dxgidebug.h>
#endif

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace Alimer
{
    namespace
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        using PFN_CREATE_DXGI_FACTORY1 = decltype(&CreateDXGIFactory1);
        static PFN_CREATE_DXGI_FACTORY1 CreateDXGIFactory1 = nullptr;

        using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
        static PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

        using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);
        static PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;

        static PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = nullptr;
#endif

#ifdef _DEBUG
        // Declare debug guids to avoid linking with "dxguid.lib"
        static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
        static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };
#endif

        static_assert(sizeof(Alimer::Viewport) == sizeof(D3D11_VIEWPORT), "Size mismatch");
        static_assert(offsetof(Alimer::Viewport, x) == offsetof(D3D11_VIEWPORT, TopLeftX), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, y) == offsetof(D3D11_VIEWPORT, TopLeftY), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, width) == offsetof(D3D11_VIEWPORT, Width), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, height) == offsetof(D3D11_VIEWPORT, Height), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, minDepth) == offsetof(D3D11_VIEWPORT, MinDepth), "Layout mismatch");
        static_assert(offsetof(Alimer::Viewport, maxDepth) == offsetof(D3D11_VIEWPORT, MaxDepth), "Layout mismatch");

        // Check for SDK Layer support.
        inline bool SdkLayersAvailable() noexcept
        {
            HRESULT hr = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
                nullptr,
                D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
                nullptr,                    // Any feature level will do.
                0,
                D3D11_SDK_VERSION,
                nullptr,                    // No need to keep the D3D device reference.
                nullptr,                    // No need to know the feature level.
                nullptr                     // No need to keep the D3D device context reference.
            );

            return SUCCEEDED(hr);
        }

        inline void SetDebugName(ID3D11DeviceChild* pObject, const std::string_view& name)
        {
            D3D_SET_OBJECT_NAME_N_A(pObject, UINT(name.size()), name.data());
        }
    }

    D3D11Graphics::D3D11Graphics(Window& window,  const GraphicsCreateInfo& createInfo)
        : Graphics(window)
        , validationMode(createInfo.validationMode)
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        HMODULE dxgiDLL = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        HMODULE d3d11DLL = LoadLibraryExW(L"d3d11.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (dxgiDLL == nullptr ||
            d3d11DLL == nullptr)
        {
            return;
        }

        CreateDXGIFactory1 = (PFN_CREATE_DXGI_FACTORY1)GetProcAddress(dxgiDLL, "CreateDXGIFactory1");
        if (CreateDXGIFactory1 == nullptr)
            return;
        CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(dxgiDLL, "CreateDXGIFactory2");
        DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)GetProcAddress(dxgiDLL, "DXGIGetDebugInterface1");

        D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(d3d11DLL, "D3D11CreateDevice");
        if (!D3D11CreateDevice) {
            return;
        }
#endif

        CreateFactory();

        // Determines whether tearing support is available for fullscreen borderless windows.
        {
            ComPtr<IDXGIFactory5> dxgiFactory5;
            if (SUCCEEDED(dxgiFactory.As(&dxgiFactory5)))
            {
                BOOL supported = 0;
                if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
                {
                    tearingSupported = (supported != 0);
                }
            }
        }

        // Get adapter and create device
        {
            UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

            if (validationMode != ValidationMode::Disabled)
            {
                if (SdkLayersAvailable())
                {
                    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
                }
                else
                {
                    LOGW("Direct3D Debug Device is not available");
                }
            }

            // Determine DirectX hardware feature levels this app will support.
            static const D3D_FEATURE_LEVEL s_featureLevels[] =
            {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
            };


            ComPtr<IDXGIAdapter1> adapter;
            GetAdapter(adapter.GetAddressOf());

            // Create the Direct3D 11 API device object and a corresponding context.
            ComPtr<ID3D11Device> device;
            ComPtr<ID3D11DeviceContext> context;

            HRESULT hr = E_FAIL;
            if (adapter)
            {
                hr = D3D11CreateDevice(
                    adapter.Get(),
                    D3D_DRIVER_TYPE_UNKNOWN,
                    nullptr,
                    creationFlags,
                    s_featureLevels,
                    _countof(s_featureLevels),
                    D3D11_SDK_VERSION,
                    device.GetAddressOf(),
                    &featureLevel,
                    context.GetAddressOf()
                );
            }
#if defined(NDEBUG)
            else
            {
                throw std::runtime_error("No Direct3D hardware device found");
            }
#else
            if (FAILED(hr))
            {
                // If the initialization fails, fall back to the WARP device.
                // For more information on WARP, see:
                // http://go.microsoft.com/fwlink/?LinkId=286690
                hr = D3D11CreateDevice(
                    nullptr,
                    D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
                    nullptr,
                    creationFlags,
                    s_featureLevels,
                    _countof(s_featureLevels),
                    D3D11_SDK_VERSION,
                    device.GetAddressOf(),
                    &featureLevel,
                    context.GetAddressOf()
                );

                if (SUCCEEDED(hr))
                {
                    OutputDebugStringA("Direct3D Adapter - WARP\n");
                }
            }
#endif

            ThrowIfFailed(hr);

            if (validationMode != ValidationMode::Disabled)
            {
                ComPtr<ID3D11Debug> d3dDebug;
                if (SUCCEEDED(device.As(&d3dDebug)))
                {
                    ComPtr<ID3D11InfoQueue> d3dInfoQueue;
                    if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
                    {
#ifdef _DEBUG
                        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
                        D3D11_MESSAGE_ID hide[] =
                        {
                            D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                        };
                        D3D11_INFO_QUEUE_FILTER filter = {};
                        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
                        filter.DenyList.pIDList = hide;
                        d3dInfoQueue->AddStorageFilterEntries(&filter);
                    }
                }
            }

            ThrowIfFailed(device.As(&d3dDevice));
            ThrowIfFailed(context.As(&d3dContext));
            //ThrowIfFailed(context.As(&m_d3dAnnotation));

            // Init caps
            D3D11_FEATURE_DATA_D3D11_OPTIONS2 options2;
            hr = device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS2, &options2, sizeof(options2));
            if (SUCCEEDED(hr) && options2.ConservativeRasterizationTier >= D3D11_CONSERVATIVE_RASTERIZATION_TIER_1)
            {
                LOGD("CONSERVATIVE_RASTERIZATION");
            }

            // Adapter info
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
            ThrowIfFailed(adapter->GetDesc1(&dxgiAdapterDesc));
            adapterInfo.name = ToUtf8(dxgiAdapterDesc.Description);
            adapterInfo.type = (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) ? GPUAdapterType::Software : GPUAdapterType::Discrete;
            adapterInfo.vendor = VendorIdToAdapterVendor(dxgiAdapterDesc.VendorId);
            adapterInfo.vendorId = dxgiAdapterDesc.VendorId;
            adapterInfo.deviceId = dxgiAdapterDesc.DeviceId;

            // Limits
            limits.maxTextureDimension1D = D3D11_REQ_TEXTURE1D_U_DIMENSION;
            limits.maxTextureDimension2D = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            limits.maxTextureDimension3D = D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            limits.maxTextureDimensionCube = D3D11_REQ_TEXTURECUBE_DIMENSION;
            limits.maxTextureArraySize = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
            limits.minConstantBufferOffsetAlignment = 256u;
            limits.minStorageBufferOffsetAlignment = 32u;
            limits.maxDrawIndirectCount = static_cast<uint32_t>(-1);
        }

        // Create SwapChain
        {
            swapChainDesc = {};
            swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = kMaxFramesInFlight;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            fullScreenDesc = {};
            fullScreenDesc.RefreshRate.Numerator = 0;
            fullScreenDesc.RefreshRate.Denominator = 1;
            fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
            fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fullScreenDesc.Windowed = !createInfo.isFullScreen;

            // Create a SwapChain from a Win32 window.
            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
                d3dDevice.Get(),
                static_cast<HWND>(window.GetPlatformHandle()),
                &swapChainDesc,
                &fullScreenDesc,
                nullptr,
                swapChain.ReleaseAndGetAddressOf()
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(dxgiFactory->MakeWindowAssociation(static_cast<HWND>(window.GetPlatformHandle()), DXGI_MWA_NO_ALT_ENTER));
#else
#endif
        }

        vsyncEnabled = createInfo.vsyncEnabled;
        AfterReset();
    }

    D3D11Graphics::~D3D11Graphics()
    {
        swapChain.Reset();
        d3dContext.Reset();

#ifdef _DEBUG
        {
            ComPtr<ID3D11Debug> d3dDebug;
            if (SUCCEEDED(d3dDevice.As(&d3dDebug)))
            {
                d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_IGNORE_INTERNAL);
            }
        }
#endif

        d3dDevice.Reset();
        dxgiFactory.Reset();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        //FreeLibrary(d3d11DLL);
       // FreeLibrary(dxgiDLL);
#endif
    }

    void D3D11Graphics::CreateFactory()
    {
#if defined(_DEBUG)
        bool debugDXGI = false;
        if (validationMode != ValidationMode::Disabled)
        {
            RefCountPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                debugDXGI = true;

                ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));

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
        }

        if (!debugDXGI)
#endif
        {
            ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf())));
        }
    }

    void D3D11Graphics::GetAdapter(IDXGIAdapter1** ppAdapter)
    {
        *ppAdapter = nullptr;

        RefCountPtr<IDXGIAdapter1> adapter;

#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
        ComPtr<IDXGIFactory6> dxgiFactory6;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory6)))
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

#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif

                break;
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

#ifdef _DEBUG
                wchar_t buff[256] = {};
                swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
                OutputDebugStringW(buff);
#endif

                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }

    void D3D11Graphics::AfterReset()
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(swapChain->GetDesc1(&swapChainDesc));

        backBufferWidth = swapChainDesc.Width;
        backBufferHeight = swapChainDesc.Height;
    }

    void D3D11Graphics::HandleDeviceLost()
    {
        // TODO:
    }

    void D3D11Graphics::WaitIdle()
    {
        d3dContext->Flush();
    }

    bool D3D11Graphics::BeginFrame()
    {
        if (deviceLost)
            return false;

        // TODO: Handle automatic resize.

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
        if (SUCCEEDED(swapChain->GetFullscreenDesc(&newFullScreenDesc)))
        {
            if (fullScreenDesc.Windowed != newFullScreenDesc.Windowed)
            {
            }
        }

        //RECT rect;
        //GetClientRect((HWND)window.handle, &rect);
#endif

        return true;
    }

    void D3D11Graphics::EndFrame()
    {
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

            frameCount++;
            frameIndex = frameCount % kMaxFramesInFlight;

            if (!dxgiFactory->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                CreateFactory();
            }
        }
    }

    void D3D11Graphics::Resize(uint32_t newWidth, uint32_t newHeight)
    {

    }

    TextureRef D3D11Graphics::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef D3D11Graphics::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef D3D11Graphics::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef D3D11Graphics::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
