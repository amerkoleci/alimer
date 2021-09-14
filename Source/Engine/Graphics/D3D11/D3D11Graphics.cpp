// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D11Graphics.h"
#include "D3D11CommandContext.h"
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

    D3D11GraphicsDevice::D3D11GraphicsDevice(Window& window, const GraphicsCreateInfo& createInfo)
        : Graphics(window)
        , validationMode(createInfo.validationMode)
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        dxgiDLL = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        d3d11DLL = LoadLibraryExW(L"d3d11.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

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

            ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&d3dDevice)));
            ID3D11DeviceContext1* immediateContext;
            ThrowIfFailed(context->QueryInterface(IID_PPV_ARGS(&immediateContext)));

            // Create main context.
            mainContext = std::make_unique<D3D11CommandContext>(this, immediateContext);

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

       

        backBufferRTVFormat = createInfo.srgb ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
        depthBufferFormat = ToDXGIFormat(createInfo.depthStencilFormat);

        // Create SwapChain
        {
            swapChainDesc = {};
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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
                d3dDevice,
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

        //D3D11_BUFFER_DESC desc{};
        //desc.ByteWidth = 28;
        //desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        //desc.MiscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        ////desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        //desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        //desc.StructureByteStride = 28;
        //ComPtr<ID3D11Buffer> buffer;
        //HRESULT hr = d3dDevice->CreateBuffer(&desc, nullptr, &buffer);

        vsyncEnabled = createInfo.vsyncEnabled;
        AfterReset();
    }

    D3D11GraphicsDevice::~D3D11GraphicsDevice()
    {
        depthStencilView.Reset();
        backBufferView.Reset();
        backBuffer.Reset();
        depthStencilTexture.Reset();
        swapChain.Reset();
        mainContext->GetHandle()->Flush();
        mainContext.reset();

        const ULONG refCount = d3dDevice->Release();
#ifdef _DEBUG
        if (refCount)
        {
            LOGD("There are {} unreleased references left on the D3D device!", refCount);

            ID3D11Debug* d3d11Debug = nullptr;
            if (SUCCEEDED(d3dDevice->QueryInterface(IID_PPV_ARGS(&d3d11Debug))))
            {
                d3d11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
                d3d11Debug->Release();
            }

        }
#else
        (void)refCount; // avoid warning
#endif
        d3dDevice = nullptr;

        dxgiFactory.Reset();

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        FreeLibrary(dxgiDLL);
        FreeLibrary(d3d11DLL);
#endif
    }

    void D3D11GraphicsDevice::CreateFactory()
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

    void D3D11GraphicsDevice::GetAdapter(IDXGIAdapter1** ppAdapter)
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

    void D3D11GraphicsDevice::AfterReset()
    {
        // Handle color space settings for HDR
        UpdateColorSpace();

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(swapChain->GetDesc1(&swapChainDesc));

        backBufferWidth = swapChainDesc.Width;
        backBufferHeight = swapChainDesc.Height;

        // Create a render target view of the swap chain back buffer.
        ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf())));

        D3D11_TEXTURE2D_DESC textureDesc;
        backBuffer->GetDesc(&textureDesc);

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Format = backBufferRTVFormat;

        ThrowIfFailed(d3dDevice->CreateRenderTargetView(
            backBuffer.Get(),
            &renderTargetViewDesc,
            backBufferView.ReleaseAndGetAddressOf()
        ));

        if (depthBufferFormat != DXGI_FORMAT_UNKNOWN)
        {
            // Create a depth stencil view for use with 3D rendering if needed.
            D3D11_TEXTURE2D_DESC depthStencilDesc = {};
            depthStencilDesc.Width = backBufferWidth;
            depthStencilDesc.Height = backBufferHeight;
            depthStencilDesc.MipLevels = 1;
            depthStencilDesc.ArraySize = 1;
            depthStencilDesc.Format = depthBufferFormat;
            depthStencilDesc.SampleDesc.Count = 1;
            depthStencilDesc.SampleDesc.Quality = 0;
            depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
            depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            ThrowIfFailed(d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilTexture.ReleaseAndGetAddressOf()));

            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
            depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            depthStencilViewDesc.Format = depthBufferFormat;
            ThrowIfFailed(d3dDevice->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, depthStencilView.ReleaseAndGetAddressOf()));
        }
    }

    void D3D11GraphicsDevice::UpdateColorSpace()
    {
        colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

        bool isDisplayHDR10 = false;

        if (swapChain)
        {
            ComPtr<IDXGIOutput> output;
            if (SUCCEEDED(swapChain->GetContainingOutput(output.GetAddressOf())))
            {
                ComPtr<IDXGIOutput6> output6;
                if (SUCCEEDED(output.As(&output6)))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    ThrowIfFailed(output6->GetDesc1(&desc));

                    if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                    {
                        // Display output is HDR10.
                        isDisplayHDR10 = true;
                    }
                }
            }
        }

        if (isDisplayHDR10)
        {
            switch (backBufferFormat)
            {
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                // The application creates the HDR10 signal.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
                break;

            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                // The system creates the HDR10 signal; application uses linear values.
                colorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
                break;

            default:
                break;
            }
        }

        ComPtr<IDXGISwapChain3> swapChain3;
        if (SUCCEEDED(swapChain.As(&swapChain3)))
        {
            UINT colorSpaceSupport = 0;
            if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport))
                && (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
            {
                ThrowIfFailed(swapChain3->SetColorSpace1(colorSpace));
            }
        }
    }

    void D3D11GraphicsDevice::HandleDeviceLost()
    {
        // Signal lost
        DeviceLost.Emit();

        depthStencilView.Reset();
        backBufferView.Reset();
        backBuffer.Reset();
        depthStencilTexture.Reset();
        swapChain.Reset();
        mainContext.reset();

        const ULONG refCount = d3dDevice->Release();
#ifdef _DEBUG
        if (refCount)
        {
            LOGD("There are {} unreleased references left on the D3D device!", refCount);

            ID3D11Debug* d3d11Debug = nullptr;
            if (SUCCEEDED(d3dDevice->QueryInterface(IID_PPV_ARGS(&d3d11Debug))))
            {
                d3d11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
                d3d11Debug->Release();
            }

        }
#else
        (void)refCount; // avoid warning
#endif
        d3dDevice = nullptr;
        dxgiFactory.Reset();

        // Signal restored
        DeviceRestored.Emit();
    }

    CommandContext* D3D11GraphicsDevice::GetImmediateContext() const
    {
        return mainContext.get();
    }

    void D3D11GraphicsDevice::WaitIdle()
    {
        mainContext->GetHandle()->Flush();
    }

    bool D3D11GraphicsDevice::BeginFrame()
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

    void D3D11GraphicsDevice::EndFrame()
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

    void D3D11GraphicsDevice::Resize(uint32_t newWidth, uint32_t newHeight)
    {
        // Clear the previous window size specific context.
        ID3D11RenderTargetView* nullViews[] = { nullptr };
        mainContext->GetHandle()->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
        backBufferView.Reset();
        depthStencilView.Reset();
        backBuffer.Reset();
        depthStencilTexture.Reset();
        mainContext->GetHandle()->Flush();

        // If the swap chain already exists, resize it.
        HRESULT hr = swapChain->ResizeBuffers(
            0,
            newWidth,
            newHeight,
            DXGI_FORMAT_UNKNOWN,
            tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
#ifdef _DEBUG
            char buff[64] = {};
            sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n",
                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3dDevice->GetDeviceRemovedReason() : hr));
            OutputDebugStringA(buff);
#endif
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method
            // and correctly set up the new device.
            return;
        }
        else
        {
            ThrowIfFailed(hr);
        }

        AfterReset();
    }

    TextureRef D3D11GraphicsDevice::CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData)
    {
        return nullptr;
    }

    SamplerRef D3D11GraphicsDevice::CreateSampler(const SamplerDesc& desc)
    {
        return nullptr;
    }
    ShaderRef D3D11GraphicsDevice::CreateShader(ShaderStages stage, const void* bytecode, size_t bytecodeLength)
    {
        return nullptr;
    }

    PipelineRef D3D11GraphicsDevice::CreateRenderPipeline(const RenderPipelineDesc& desc)
    {
        return nullptr;
    }
}
