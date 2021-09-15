// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Graphics.h"
#include "D3D12CommandQueue.h"
#include "dxcapi.h"
#include <queue>
#include <deque>

namespace Alimer
{
	struct D3D12UploadContext
	{
		ID3D12GraphicsCommandList* commandList;
		void* CPUAddress = nullptr;
		uint64_t resourceOffset = 0;
		ID3D12Resource* resource = nullptr;
		void* submission = nullptr;
	};

	class D3D12Graphics final : public Graphics
	{
	public:
		D3D_FEATURE_LEVEL featureLevel{};
		bool supportsRenderPass{ false };

	public:
		static bool IsAvailable();

		D3D12Graphics(ValidationMode validationMode);
		~D3D12Graphics() override;

		void WaitIdle() override;
		void FinishFrame() override;

		D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
		void FreeDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_CPU_DESCRIPTOR_HANDLE handle);
        uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

		void SyncCopyQueue();

		void DeferDestroy(IUnknown* resource, D3D12MA::Allocation* allocation = nullptr);
		ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc);

		// Resource upload/init
		D3D12UploadContext ResourceUploadBegin(uint64_t size);
		void ResourceUploadEnd(D3D12UploadContext& context);

		void HandleDeviceLost(HRESULT hr);

		IDXGIFactory4* GetDXGIFactory() const noexcept { return dxgiFactory.Get(); }
		bool IsTearingSupported() const noexcept { return tearingSupported; }
		auto GetHandle() const noexcept { return device; }
		auto GetAllocator() const noexcept { return allocator; }
		auto GetDxcUtils() const noexcept { return dxcUtils.Get(); }

		void* GetNativeHandle() const noexcept override { return device; }

		D3D12CommandQueue* GetD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE type) const
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return ToD3D12(computeQueue);
			default:
				return ToD3D12(graphicsQueue);
			}
		}

		ID3D12CommandQueue* GetCommandQueue() const
		{
			return ToD3D12Handle(graphicsQueue);
		}

		// Test to see if a fence has already been reached
		bool IsFenceComplete(uint64_t fenceValue)
		{
			return GetD3D12CommandQueue(D3D12_COMMAND_LIST_TYPE(fenceValue >> 56))->IsFenceComplete(fenceValue);
		}

		D3D12DescriptorAlloc AllocateSRV();
        ID3D12DescriptorHeap* GetResourceDescriptorHeap() const
        {
            return resourceHeap.handle;
        }

        const D3D12_GPU_DESCRIPTOR_HANDLE& GetResourceDescriptorHeapGPUStart() const
        {
            return resourceHeap.GPUStart;
        }

        ID3D12DescriptorHeap* GetSamplerDescriptorHeap() const
        {
            return samplerHeap.handle;
        }

        const D3D12_GPU_DESCRIPTOR_HANDLE& GetSamplerDescriptorHeapGPUStart() const
        {
            return samplerHeap.GPUStart;
        }

	private:
		void ProcessDeletionQueue();

		void GetAdapter(IDXGIAdapter1** ppAdapter);
        TextureRef CreateTextureCore(const TextureCreateInfo& info, const void* initialData) override;
		BufferRef CreateBuffer(const BufferCreateInfo& info, const void* initialData) override;
        ShaderRef CreateShader(ShaderStages stage, const std::vector<uint8_t>& byteCode, const std::string& entryPoint) override;
        SamplerRef CreateSampler(const SamplerCreateInfo* info) override;
		PipelineRef CreateRenderPipeline(const RenderPipelineStateCreateInfo* info) override;
		PipelineRef CreateComputePipeline(const ComputePipelineCreateInfo* info) override;
        SwapChainRef CreateSwapChain(void* windowHandle, const SwapChainCreateInfo& info) override;

#ifdef _DEBUG
		void PrintAdapterInformation(IDXGIAdapter1* adapter);
#endif

		ComPtr<IDxcUtils> dxcUtils;

		DWORD dxgiFactoryFlags = 0u;
		ComPtr<IDXGIFactory4> dxgiFactory;
		bool tearingSupported = false;

		bool shuttingDown = false;
		ID3D12Device2* device = nullptr;
		D3D12MA::Allocator* allocator = nullptr;

		ID3D12Fence* frameFence = nullptr;
		HANDLE frameFenceEvent = nullptr;

		// Upload data
		static constexpr uint64_t kUploadBufferSize = 96 * 1024 * 1024;
		static constexpr uint64_t kMaxUploadSubmissions = 16;

		struct UploadSubmission
		{
			ID3D12CommandAllocator* commandAllocator = nullptr;
			ID3D12GraphicsCommandList1* commandList = nullptr;
			uint64_t offset = 0;
			uint64_t size = 0;
			uint64_t fenceValue = 0;
			uint64_t padding = 0;

			void Reset()
			{
				offset = 0;
				size = 0;
				fenceValue = 0;
				padding = 0;
			}
		};

		ID3D12CommandQueue* uploadCmdQueue;
		ID3D12Fence* uploadFence;
		HANDLE uploadFenceEvent;
		uint64_t uploadFenceValue = 0;

		D3D12MA::Allocation* uploadBufferAllocation = nullptr;
		ID3D12Resource* uploadBuffer = nullptr;
		uint8_t* uploadBufferCPUAddr = nullptr;
		SRWLOCK uploadSubmissionLock = SRWLOCK_INIT;
		SRWLOCK uploadQueueLock = SRWLOCK_INIT;

		// These are protected by UploadSubmissionLock
		uint64_t uploadBufferStart = 0;
		uint64_t uploadBufferUsed = 0;
		UploadSubmission uploadSubmissions[kMaxUploadSubmissions] = {};
		uint64_t uploadSubmissionStart = 0;
		uint64_t uploadSubmissionUsed = 0;

		void ClearFinishedUploads(uint64_t flushCount);
		UploadSubmission* AllocUploadSubmission(uint64_t size);

		std::mutex destroyMutex;
		std::deque<std::pair<D3D12MA::Allocation*, uint64_t>> deferredAllocations;
		std::deque<std::pair<IUnknown*, uint64_t>> deferredReleases;

		struct DescriptorAllocator
		{
			D3D12Graphics* device = nullptr;
			std::mutex locker;
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			std::vector<ID3D12DescriptorHeap*> heaps;
			uint32_t descriptorSize = 0;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> freeList;

			void Init(D3D12Graphics* device_, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
			{
				device = device_;
				desc.Type = type;
				desc.NumDescriptors = numDescriptors;
				descriptorSize = device->device->GetDescriptorHandleIncrementSize(type);
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
				ThrowIfFailed(device->device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heaps.back())));

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

		DescriptorAllocator rtv;
		DescriptorAllocator dsv;
		DescriptorAllocator resourceDescriptorAllocator;
        DescriptorAllocator samplerDescriptorAllocator;

		struct DescriptorHeap
		{
			ID3D12DescriptorHeap* handle = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE CPUStart;
			D3D12_GPU_DESCRIPTOR_HANDLE GPUStart;

            // CPU status:
            std::atomic<uint64_t> allocationOffset{ 0 };

            // GPU status:
            ID3D12Fence* fence = nullptr;
            uint64_t fenceValue = 0;
            uint64_t cachedCompletedValue = 0;
		};
        DescriptorHeap resourceHeap;
        DescriptorHeap samplerHeap;

		// Bindless
        static constexpr uint32_t kBindlessResourceCapacity = 500000;
        static constexpr uint32_t kBindlessSamplerCapacity = 256;

        std::vector<uint32_t> freeBindlessResources;
        std::vector<uint32_t> freeBindlessSamplers;
		SRWLOCK bindlessFreeLock = SRWLOCK_INIT;
	};
}
