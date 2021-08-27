// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "RHI.h"
#include "PlatformInclude.h"
//#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#   include <dxgidebug.h>
#endif

#include <unordered_map>

namespace alimer::rhi
{
    struct D3D11_ViewKey
    {
        Format format = Format::Undefined;
        TextureSubresourceSet set;
        bool isReadOnlyDSV = false;

        D3D11_ViewKey()
        {
        }

        D3D11_ViewKey(const TextureSubresourceSet& set_, alimer::rhi::Format format_, bool isReadOnlyDSV_ = false)
            : set(set_)
            , format(format_)
            , isReadOnlyDSV(isReadOnlyDSV_)
        {
        }

        bool operator== (const D3D11_ViewKey& rhs) const
        {
            return format == rhs.format && set == rhs.set && isReadOnlyDSV == rhs.isReadOnlyDSV;
        }
    };

    class D3D11_Device;

    class D3D11_Texture final : public RefCounter<ITexture>
    {
    public:
        D3D11_Device* device;
        TextureDesc desc;
        DXGI_FORMAT dxgiFormat;
        RefCountPtr<ID3D11Resource> handle;

        D3D11_Texture(D3D11_Device* device_, void* nativeHandle, const TextureDesc& desc_, const TextureData* initialData);
        ~D3D11_Texture() override;

        IDevice* GetDevice() const override;
        const TextureDesc& GetDesc() const override { return desc; }
        //uint64_t GetAllocatedSize() const override { return allocatedSize; }

        ID3D11RenderTargetView* GetRTV(uint32_t mipLevel = 0, uint32_t slice = 0, uint32_t arraySize = 1);
        ID3D11DepthStencilView* GetDSV(uint32_t mipLevel = 0, uint32_t slice = 0, uint32_t arraySize = 1, bool isReadOnly = false);

    private:
        void ApiSetName(const std::string_view& newName) override;

        struct ViewInfoHashFunc
        {
            std::size_t operator()(const D3D11_ViewKey& key) const
            {
                size_t hash = 0;
                alimer::rhi::hash_combine(hash, static_cast<uint32_t>(key.format));
                alimer::rhi::hash_combine(hash, key.set);
                alimer::rhi::hash_combine(hash, key.isReadOnlyDSV);
                return hash;
            }
        };

        std::unordered_map<D3D11_ViewKey, RefCountPtr<ID3D11ShaderResourceView>, ViewInfoHashFunc> shaderResourceViews;
        std::unordered_map<D3D11_ViewKey, RefCountPtr<ID3D11RenderTargetView>, ViewInfoHashFunc> renderTargetViews;
        std::unordered_map<D3D11_ViewKey, RefCountPtr<ID3D11DepthStencilView>, ViewInfoHashFunc> depthStencilViews;
        std::unordered_map<D3D11_ViewKey, RefCountPtr<ID3D11UnorderedAccessView>, ViewInfoHashFunc> unorderedAccessViews;
    };

    class D3D11_Shader final : public RefCounter<IShader>
    {
    public:
        IDevice* device = nullptr;
        ShaderStages stage = ShaderStages::None;
        RefCountPtr<ID3D11VertexShader> VS;
        RefCountPtr<ID3D11HullShader> HS;
        RefCountPtr<ID3D11DomainShader> DS;
        RefCountPtr<ID3D11GeometryShader> GS;
        RefCountPtr<ID3D11PixelShader> PS;
        RefCountPtr<ID3D11ComputeShader> CS;
        std::vector<char> bytecode;

        IDevice* GetDevice() const override { return device; }
        ShaderStages GetStage() const override { return stage; }
        void ApiSetName(const std::string_view& newName) override;
    };

    class D3D11_Pipeline : public RefCounter<IPipeline>
    {
    public:
        IDevice* device = nullptr;
        ShaderStages shaderStages = ShaderStages::None;
        RefCountPtr<ID3D11VertexShader> vertex;
        RefCountPtr<ID3D11HullShader> hull;
        RefCountPtr<ID3D11DomainShader> domain;
        RefCountPtr<ID3D11GeometryShader> geometry;
        RefCountPtr<ID3D11PixelShader> pixel;

        ID3D11InputLayout* inputLayout = nullptr;
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

        IDevice* GetDevice() const override { return device; }
        void ApiSetName(const std::string_view& newName) override;
    };

    class D3D11_CommandList final : public ICommandList
    {
    private:
        IDevice* device;
        RefCountPtr<ID3D11DeviceContext1>       context;
        RefCountPtr<ID3DUserDefinedAnnotation>  annotation;
        RenderPassDesc currentPass = {};

    public:
        D3D11_CommandList(IDevice* device_, ID3D11DeviceContext* context_);

        void PushDebugGroup(const std::string_view& name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const std::string_view& name) override;

        void BeginDefaultRenderPass(const Color& clearColor, bool clearDepth = true, bool clearStencil = true, float depth = 1.0f, uint8_t stencil = 0) override;
        void BeginRenderPass(const RenderPassDesc& desc) override;
        void EndRenderPass() override;

        void SetPipeline(_In_ IPipeline* pipeline) override;
        void BindRenderPipeline(const D3D11_Pipeline* pipeline);
        void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) override;
    };

    class D3D11_Device final : public RefCounter<IDevice>
    {
    private:
        RefCountPtr<IDXGIFactory2> dxgiFactory;
        bool tearingSupported{ false };
        bool deviceLost{ false };

        DXGI_SWAP_CHAIN_DESC1                       swapChainDesc{};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC             fullScreenDesc{};
#endif

        RefCountPtr<ID3D11Device1>          d3dDevice;
        D3D_FEATURE_LEVEL                   featureLevel{};
        RefCountPtr<ID3D11DeviceContext1>   immediateContext;
        std::unique_ptr<D3D11_CommandList>  commandList;

        RefCountPtr<IDXGISwapChain1> swapChain;
        TextureHandle backBuffer;
        Format depthStencilFormat = Format::Undefined;
        TextureHandle depthStencilTexture;

        void CreateFactory();
        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void HandleDeviceLost();

    public:
        D3D11_Device();
        ~D3D11_Device() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters);
        void WaitIdle() override;
        ICommandList* BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;
        void AfterReset();

        auto GetD3DDevice() const noexcept { return d3dDevice.Get(); }

        ITexture* GetCurrentBackBuffer() const override { return backBuffer; }

        ITexture* GetBackBuffer(uint32_t index) const override
        {
            if (index == 0)
                return backBuffer;

            return nullptr;
        }

        uint32_t GetCurrentBackBufferIndex() const override { return 0; }
        uint32_t GetBackBufferCount() const override { return 1; }
        ITexture* GetBackBufferDepthStencilTexture() const override { return depthStencilTexture; }

        TextureHandle CreateTexture(const TextureDesc& desc, const TextureData* initialData = nullptr) override;
        TextureHandle CreateExternalTexture(void* nativeHandle, const TextureDesc& desc) override;
        ShaderHandle CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main") override;
        std::vector<uint8_t> CompileShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");

        PipelineHandle CreateRenderPipeline(const RenderPipelineDesc& desc) override;

    private:
#if !defined(ALIMER_DISABLE_SHADER_COMPILER) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        bool D3DCompiler_LoadFailed = false;
        HINSTANCE D3DCompiler = nullptr;
        bool LoadShaderCompiler();
#endif
    };
}
