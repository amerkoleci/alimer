// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/Pipeline.h"
#include "PlatformInclude.h"
#include <dxgi1_6.h>

#include "directx/d3d12.h"
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemAlloc.h"

#ifdef _DEBUG
#   include <dxgidebug.h>
#endif

#include <array>
#include <deque>
#include <unordered_map>

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace Alimer::rhi
{
    struct D3D12_ViewKey
    {
        PixelFormat format = PixelFormat::Undefined;
        TextureSubresourceSet set;
        bool isReadOnlyDSV = false;

        D3D12_ViewKey()
        {
        }

        D3D12_ViewKey(const TextureSubresourceSet& set_, PixelFormat format_, bool isReadOnlyDSV_ = false)
            : set(set_)
            , format(format_)
            , isReadOnlyDSV(isReadOnlyDSV_)
        {
        }

        bool operator== (const D3D12_ViewKey& rhs) const
        {
            return format == rhs.format && set == rhs.set && isReadOnlyDSV == rhs.isReadOnlyDSV;
        }
    };

    class D3D12_Device;

    class D3D12_Texture final : public Texture
    {
    public:
        D3D12_Device* device;
        DXGI_FORMAT dxgiFormat;
        ID3D12Resource* handle = nullptr;
        D3D12MA::Allocation* allocation = nullptr;
        u64 allocatedSize{ 0 };

        D3D12_Texture(const TextureDesc& info);
        ~D3D12_Texture() override;

        uint64_t GetAllocatedSize() const override { return allocatedSize; }
        auto GetHandle() const noexcept { return handle; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint32_t mipLevel = 0, uint32_t slice = 0, uint32_t arraySize = 1);
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(uint32_t mipLevel = 0, uint32_t slice = 0, uint32_t arraySize = 1, bool isReadOnly = false);

    private:
        //uint64_t GetAllocatedSize() const override { return allocatedSize; }

        struct ViewInfoHashFunc
        {
            std::size_t operator()(const D3D12_ViewKey& key) const
            {
                size_t hash = 0;
                Alimer::HashCombine(hash, static_cast<u32>(key.format));
                Alimer::HashCombine(hash, key.set);
                Alimer::HashCombine(hash, key.isReadOnlyDSV);
                return hash;
            }
        };

        std::atomic_uint32_t refCount = 1;
        std::unordered_map<D3D12_ViewKey, D3D12_CPU_DESCRIPTOR_HANDLE, ViewInfoHashFunc> shaderResourceViews;
        std::unordered_map<D3D12_ViewKey, D3D12_CPU_DESCRIPTOR_HANDLE, ViewInfoHashFunc> renderTargetViews;
        std::unordered_map<D3D12_ViewKey, D3D12_CPU_DESCRIPTOR_HANDLE, ViewInfoHashFunc> depthStencilViews;
        std::unordered_map<D3D12_ViewKey, D3D12_CPU_DESCRIPTOR_HANDLE, ViewInfoHashFunc> unorderedAccessViews;
    };

    class D3D12_Buffer final : public Buffer
    {
    public:
        D3D12_Device* device;
        ID3D12Resource* handle = nullptr;
        D3D12MA::Allocation* allocation = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS deviceAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        uint8_t* mappedData = nullptr;
        u64 allocatedSize{ 0 };

        D3D12_Buffer(const BufferDesc& desc);
        ~D3D12_Buffer() override;

        uint64_t GetAllocatedSize() const override { return allocatedSize; }
        uint64_t GetDeviceAddress() const override { return deviceAddress; }
        uint8_t* MappedData() const override { return mappedData; }
    };

    class D3D12_Sampler final : public Sampler
    {
    public:
        D3D12_Device* device;
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor = {};
        u32 bindlessIndex = kInvalidBindlessIndex;

        D3D12_Sampler(const SamplerDesc& desc);
        ~D3D12_Sampler() override;
        u32 GetBindlessIndex() const override { return bindlessIndex; }
    };

    class D3D12_Shader final : public Shader
    {
    public:
        D3D12_Shader(ShaderStages stage);
        ShaderStages stage = ShaderStages::None;
        std::vector<uint8_t> bytecode;
    };

    class D3D12_Pipeline : public Pipeline
    {
    public:
        D3D12_Device* device;
        D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        uint32_t vboSlotsUsed = 0;
        std::array<uint32_t, kMaxVertexBufferBindings> vboStrides = {};

        ID3D12RootSignature* rootSignature = nullptr;
        ID3D12PipelineState* handle = nullptr;

        D3D12_Pipeline(Type type);
        ~D3D12_Pipeline() override;

    private:
    };

    class D3D12_CommandList final : public ICommandList
    {
    private:
        D3D12_Device* device;
        CommandQueue queue;
        RefCountPtr<ID3D12CommandAllocator> commandAllocators[kMaxFramesInFlight];
        RefCountPtr<ID3D12GraphicsCommandList4> handle;
        RenderPassDesc currentPass = {};

        std::vector<D3D12_RESOURCE_BARRIER> barriers;
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rtvDescs[kMaxColorAttachments] = {};
        D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS subresourceParameters[kMaxColorAttachments] = {};

        D3D12_Pipeline* boundPipeline = nullptr;
        D3D12_PRIMITIVE_TOPOLOGY currentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        std::array<D3D12_VERTEX_BUFFER_VIEW, kMaxVertexBufferBindings> currentVbos = {};

    public:
        D3D12_CommandList(D3D12_Device* device_, CommandQueue queue_);

        void Reset(uint32_t frameIndex);
        ID3D12CommandList* Commit();
        void FlushBarriers();

        void PushDebugGroup(const std::string_view& name) override;
        void PopDebugGroup() override;
        void InsertDebugMarker(const std::string_view& name) override;

        void BeginDefaultRenderPass(const Color& clearColor, bool clearDepth = true, bool clearStencil = true, float depth = 1.0f, uint8_t stencil = 0) override;
        void BeginRenderPass(const RenderPassDesc& desc) override;
        void EndRenderPass() override;

        void SetPipeline(_In_ Pipeline* pipeline) override;

        void SetVertexBuffer(uint32_t index, const Buffer* buffer) override;
        void SetIndexBuffer(const Buffer* buffer, uint64_t offset, IndexType indexType) override;
        void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t baseInstance = 0) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t baseInstance) override;

        void BindRenderPipeline();
        void FlushDraw();
    };

    struct D3D12CopyAllocator final
    {
        D3D12_Device* device = nullptr;
        RefCountPtr<ID3D12CommandQueue> queue;
        std::mutex locker;

        struct CopyCMD
        {
            RefCountPtr<ID3D12CommandAllocator> commandAllocator;
            RefCountPtr<ID3D12GraphicsCommandList> commandList;
            RefCountPtr<ID3D12Fence> fence;
            BufferRef uploadBuffer;
        };
        std::vector<CopyCMD> freeList;

        D3D12CopyAllocator(D3D12_Device* device);
        ~D3D12CopyAllocator();
        CopyCMD Allocate(u64 size);
        void Submit(CopyCMD cmd);
    };

    class D3D12_Device final : public Graphics
    {
    private:
        DWORD dxgiFactoryFlags = 0;
        RefCountPtr<IDXGIFactory4> dxgiFactory;
        bool tearingSupported{ false };
        bool deviceLost{ false };

        DXGI_SWAP_CHAIN_DESC1                       swapChainDesc{};
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC             fullScreenDesc{};
#endif

        RefCountPtr<ID3D12Device5>          d3dDevice;
        D3D_FEATURE_LEVEL                   featureLevel{};
        D3D12MA::Allocator*                 allocator = nullptr;

        struct Queue
        {
            RefCountPtr<ID3D12CommandQueue> queue;
            RefCountPtr<ID3D12Fence> fence;
            RefCountPtr<ID3D12Fence> frameFence[kMaxFramesInFlight];
            ID3D12CommandList* submitCommandLists[kMaxCommandLists] = {};
            uint32_t submitCount = 0;
        } queues[(uint8_t)CommandQueue::Count];

        std::unique_ptr<D3D12CopyAllocator> copyAllocator;

        struct DescriptorAllocator
        {
            D3D12_Device* device = nullptr;
            std::mutex locker;
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            std::vector<ID3D12DescriptorHeap*> heaps;
            uint32_t descriptorSize = 0;
            std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> freeList;

            void Init(D3D12_Device* device_, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
            {
                device = device_;
                desc.Type = type;
                desc.NumDescriptors = numDescriptors;
                descriptorSize = device->d3dDevice->GetDescriptorHandleIncrementSize(type);
            }

            void Shutdown()
            {
                for (auto heap : heaps)
                {
                    heap->Release();
                }
                heaps.clear();
            }

            void BlockAllocate()
            {
                heaps.emplace_back();
                ThrowIfFailed(device->d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heaps.back())));

                D3D12_CPU_DESCRIPTOR_HANDLE heap_start = heaps.back()->GetCPUDescriptorHandleForHeapStart();
                for (UINT i = 0; i < desc.NumDescriptors; ++i)
                {
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_start;
                    handle.ptr += i * descriptorSize;
                    freeList.push_back(handle);
                }
            }

            D3D12_CPU_DESCRIPTOR_HANDLE Allocate()
            {
                locker.lock();
                if (freeList.empty())
                {
                    BlockAllocate();
                }

                ALIMER_ASSERT(!freeList.empty());

                D3D12_CPU_DESCRIPTOR_HANDLE handle = freeList.back();
                freeList.pop_back();
                locker.unlock();
                return handle;
            }

            void Free(D3D12_CPU_DESCRIPTOR_HANDLE index)
            {
                locker.lock();
                freeList.push_back(index);
                locker.unlock();
            }
        };

        DescriptorAllocator resourceDescriptorAllocator;
        DescriptorAllocator samplerDescriptorAllocator;
        DescriptorAllocator rtvDescriptorAllocator;
        DescriptorAllocator dsvDescriptorAllocator;

        struct DescriptorHeap
        {
            RefCountPtr<ID3D12DescriptorHeap> handle;
            D3D12_CPU_DESCRIPTOR_HANDLE CPUStart;
            D3D12_GPU_DESCRIPTOR_HANDLE GPUStart;

            // CPU status:
            std::atomic_uint64_t allocationOffset{ 0 };

            // GPU status:
            RefCountPtr<ID3D12Fence> fence;
            uint64_t fenceValue = 0;
            uint64_t cachedCompletedValue = 0;
        };
        DescriptorHeap resourceHeap;
        DescriptorHeap samplerHeap;

        std::vector<u32> freeBindlessResources;
        std::vector<u32> freeBindlessSamplers;

        // Bindless
        static constexpr u32 kD3D12BindlessResourceCapacity = 500000;
        static constexpr u32 kD3D12BindlessSamplerCapacity = 256;

        struct CommandListMetadata
        {
            CommandQueue queue = {};
            std::vector<uint8_t> waits;
        } commandListMeta[kMaxCommandLists];
        std::atomic_uint8_t commandListCount{ 0 };

        std::unique_ptr<D3D12_CommandList> commandLists[kMaxCommandLists][(uint8_t)CommandQueue::Count];
        inline D3D12_CommandList* GetCommandList(uint8_t cmd)
        {
            return commandLists[cmd][(uint8_t)commandListMeta[cmd].queue].get();
        }

        uint64_t frameCount = 0;
        uint32_t frameIndex = 0;

        RefCountPtr<IDXGISwapChain3> swapChain;
        std::vector<TextureRef> backBuffers;
        PixelFormat depthStencilFormat = PixelFormat::Undefined;
        TextureRef depthStencilTexture;

        // Destroy logic
        bool shuttingDown = false;
        SRWLOCK destroyMutex = SRWLOCK_INIT;
        std::deque<std::pair<D3D12MA::Allocation*, uint64_t>> deferredAllocations;
        std::deque<std::pair<IUnknown*, uint64_t>> deferredReleases;
        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessResources;
        std::deque<std::pair<uint32_t, uint64_t>> destroyedBindlessSamplers;

        void GetAdapter(IDXGIAdapter1** ppAdapter);
        void HandleDeviceLost();
        void ProcessDeletionQueue();

    public:
        [[nodiscard]] static bool IsAvailable();

        D3D12_Device(ValidationMode validationMode);
        ~D3D12_Device() override;

        bool Initialize(_In_ Window* window, const PresentationParameters& presentationParameters);
        void WaitIdle() override;
        void DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation = nullptr);

        ICommandList* BeginCommandList(CommandQueue queue = CommandQueue::Graphics);
        void SubmitCommandLists();

        ICommandList* BeginFrame() override;
        void EndFrame() override;
        void Resize(uint32_t newWidth, uint32_t newHeight) override;
        void AfterReset();

        auto GetD3DDevice() const noexcept { return d3dDevice.Get(); }
        auto GetGraphicsQueue() const noexcept { return queues[(uint8_t)CommandQueue::Graphics].queue.Get(); }
        auto GetComputeQueue() const noexcept { return queues[(uint8_t)CommandQueue::Compute].queue.Get(); }

        D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
        void FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_CPU_DESCRIPTOR_HANDLE handle);
        uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

        uint32_t AllocateBindlessResource(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        uint32_t AllocateBindlessSampler(D3D12_CPU_DESCRIPTOR_HANDLE handle);
        void FreeBindlessResource(uint32_t index);
        void FreeBindlessSampler(uint32_t index);

        ID3D12DescriptorHeap* GetResourceDescriptorHeap() const { return resourceHeap.handle.Get(); }
        ID3D12DescriptorHeap* GetSamplerDescriptorHeap() const { return samplerHeap.handle.Get(); }

        GraphicsAPI GetGraphicsAPI() const override { return GraphicsAPI::D3D12; }
        uint64_t GetFrameCount() const override { return frameCount; }
        uint32_t GetFrameIndex() const { return frameIndex; }

        Texture* GetCurrentBackBuffer() const override;

        Texture* GetBackBuffer(uint32_t index) const override
        {
            if (index < backBuffers.size())
            {
                return backBuffers[index].Get();
            }

            return nullptr;
        }

        uint32_t GetCurrentBackBufferIndex() const override
        {
            return swapChain->GetCurrentBackBufferIndex();
        }

        uint32_t GetBackBufferCount() const override
        {
            return swapChainDesc.BufferCount;
        }

        Texture* GetBackBufferDepthStencilTexture() const override { return depthStencilTexture; }

        TextureRef CreateTexture(const TextureDesc& desc, void* nativeHandle, const TextureData* initialData) override;
        BufferRef CreateBuffer(const BufferDesc& desc, const void* initialData) override;
        SamplerRef CreateSampler(const SamplerDesc& desc) override;
        ShaderRef CreateShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main") override;
        std::vector<uint8_t> CompileShader(ShaderStages stage, const std::string& source, const std::string& entryPoint = "main");
        PipelineRef CreateRenderPipeline(const RenderPipelineDesc& desc) override;

    private:

#if !defined(ALIMER_DISABLE_SHADER_COMPILER) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        bool D3DCompiler_LoadFailed = false;
        HINSTANCE D3DCompiler = nullptr;
        bool LoadShaderCompiler();
#endif
    };
}
