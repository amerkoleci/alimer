// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ALIMER_RHI_D3D11)
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Window.h"
#include "Core/Log.h"
#include "PlatformInclude.h"
#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <dxgi1_6.h>

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

namespace alimer
{
    namespace
    {
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
    }

    static struct
    {
        RefCountPtr<ID3D11Device1>              device;
        RefCountPtr<ID3D11DeviceContext1>       context;
        RefCountPtr<ID3DUserDefinedAnnotation>  annotation;
        D3D_FEATURE_LEVEL                       featureLevel{};
    } d3d11;

    class D3D11_Texture final : public RefCounter<Texture>
    {
    public:
        RefCountPtr<ID3D11Resource> handle;

        D3D11_Texture(u32 width, u32 height)
        {
            D3D11_TEXTURE2D_DESC desc = { 0 };
            desc.Width = width;
            desc.Height = height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            HRESULT hr = d3d11.device->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)handle.ReleaseAndGetAddressOf());
            if (FAILED(hr))
            {
                return;
            }

        }

        ~D3D11_Texture() override
        {

        }
    };

    class D3D11_Graphics final : public Graphics
    {
    private:
        RefCountPtr<IDXGIFactory2> dxgiFactory;
        bool tearingSupported{ false };
        bool deviceLost{ false };

        DXGI_SWAP_CHAIN_DESC1                       swapChainDesc{};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC             fullScreenDesc{};
#endif
        bool vsyncEnabled = false;

        RefCountPtr<IDXGISwapChain1> swapChain;
        RefCountPtr<ID3D11Texture2D> renderTarget;
        RefCountPtr<ID3D11RenderTargetView> renderTargetView;
        D3D11_VIEWPORT viewport = {};

        void CreateFactory();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void HandleDeviceLost();

    public:
        D3D11_Graphics();
        ~D3D11_Graphics() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters) override;
        bool BeginFrame() override;
        void EndFrame() override;
        void Resize(u32 newWidth, u32 newHeight) override;

        void BeginDefaultRenderPass(const Color& clearColor) override;
        void EndRenderPass() override;

        RefCountPtr<Texture> CreateTexture(u32 width, u32 height) override;
    };

    D3D11_Graphics::D3D11_Graphics()
    {
        CreateFactory();

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

    D3D11_Graphics::~D3D11_Graphics()
    {
        renderTargetView.Reset();
        renderTarget.Reset();
        //m_depthStencil.Reset();
        swapChain.Reset();

        d3d11.annotation.Reset();
        d3d11.context.Reset();
        d3d11.device.Reset();
    }

    bool D3D11_Graphics::Initialize(_In_ Window* window, const PresentationParameters& presentationParameters)
    {
        RefCountPtr<IDXGIAdapter1> adapter;
        GetAdapter(adapter.GetAddressOf());

        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        if (presentationParameters.validationMode != ValidationMode::Disabled)
        {
            if (SdkLayersAvailable())
            {
                // If the project is in a debug build, enable debugging via SDK Layers with this flag.
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
            else
            {
                OutputDebugStringA("WARNING: Direct3D Debug Device is not available\n");
            }
        }

        static const D3D_FEATURE_LEVEL s_featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        // Create the Direct3D 11 API device object and a corresponding context.
        RefCountPtr<ID3D11Device> device;
        RefCountPtr<ID3D11DeviceContext> context;

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
                &d3d11.featureLevel,
                context.GetAddressOf()
            );
        }
#if defined(NDEBUG)
        else
        {
            LOGE("No Direct3D11 hardware device found");
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
                &d3d11.featureLevel,
                context.GetAddressOf()
            );

            if (SUCCEEDED(hr))
            {
                OutputDebugStringA("Direct3D Adapter - WARP\n");
            }
        }
#endif

        ThrowIfFailed(hr);

#ifndef NDEBUG
        RefCountPtr<ID3D11Debug> d3dDebug;
        if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3dDebug))))
        {
            RefCountPtr<ID3D11InfoQueue> d3dInfoQueue;
            if (SUCCEEDED(d3dDebug->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue))))
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
#endif

        ThrowIfFailed(device->QueryInterface(IID_PPV_ARGS(&d3d11.device)));
        ThrowIfFailed(context->QueryInterface(IID_PPV_ARGS(&d3d11.context)));
        ThrowIfFailed(context->QueryInterface(IID_PPV_ARGS(&d3d11.annotation)));

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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
            fullScreenDesc = {};
            fullScreenDesc.RefreshRate.Numerator = 0;
            fullScreenDesc.RefreshRate.Denominator = 1;
            fullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
            fullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fullScreenDesc.Windowed = !presentationParameters.isFullScreen;

            // Create a SwapChain from a Win32 window.
            ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
                d3d11.device.Get(),
                static_cast<HWND>(window->GetPlatformHandle()),
                &swapChainDesc,
                &fullScreenDesc,
                nullptr,
                swapChain.ReleaseAndGetAddressOf()
            ));

            // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
            ThrowIfFailed(dxgiFactory->MakeWindowAssociation(static_cast<HWND>(window->GetPlatformHandle()), DXGI_MWA_NO_ALT_ENTER));
#else
#endif
        }

        vsyncEnabled = presentationParameters.vsyncEnabled;

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        ThrowIfFailed(swapChain->GetDesc1(&swapChainDesc));

        // Create a render target view of the swap chain back buffer.
        ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(renderTarget.ReleaseAndGetAddressOf())));
        ThrowIfFailed(d3d11.device->CreateRenderTargetView(renderTarget.Get(), nullptr, renderTargetView.ReleaseAndGetAddressOf()));
        viewport.Width = static_cast<FLOAT>(swapChainDesc.Width);
        viewport.Height = static_cast<FLOAT>(swapChainDesc.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        return true;
    }

    bool D3D11_Graphics::BeginFrame()
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

        d3d11.context->OMSetRenderTargets(1, &renderTargetView, nullptr);
        d3d11.context->RSSetViewports(1, &viewport);

        const float clearColor[4] = { 0.392156899f, 0.584313750f, 0.929411829f, 1.0f };
        d3d11.context->ClearRenderTargetView(renderTargetView.Get(), clearColor);

        return true;
    }

    void D3D11_Graphics::EndFrame()
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
                static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? d3d11.device->GetDeviceRemovedReason() : hr));
            OutputDebugStringA(buff);
#endif
            HandleDeviceLost();
        }
        else
        {
            ThrowIfFailed(hr);

            if (!dxgiFactory->IsCurrent())
            {
                // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
                CreateFactory();
            }
        }
    }

    void D3D11_Graphics::Resize(u32 newWidth, u32 newHeight)
    {

    }

    void D3D11_Graphics::BeginDefaultRenderPass(const Color& clearColor)
    {
    }

    void D3D11_Graphics::EndRenderPass()
    {

    }

    RefCountPtr<Texture> D3D11_Graphics::CreateTexture(u32 width, u32 height)
    {
        auto result = new D3D11_Texture(width, height);

        if (result->handle)
            return TextureRef::Create(result);

        delete result;
        return nullptr;
    }

    void D3D11_Graphics::CreateFactory()
    {
#if defined(_DEBUG) && (_WIN32_WINNT >= 0x0603 /*_WIN32_WINNT_WINBLUE*/)
        bool debugDXGI = false;
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

    void D3D11_Graphics::GetAdapter(IDXGIAdapter1** ppAdapter)
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

    void D3D11_Graphics::HandleDeviceLost()
    {
        // TODO
    }

    bool D3D11_Initialize(Window* window, const PresentationParameters& presentationParameters)
    {
        gGraphics().Start<D3D11_Graphics>();
        return gGraphics().Initialize(window, presentationParameters);
    }
}

#endif /* defined(ALIMER_RHI_D3D11) */
