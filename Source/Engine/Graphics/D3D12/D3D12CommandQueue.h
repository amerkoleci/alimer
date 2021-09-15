// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandQueue.h"
#include "D3D12Utils.h"
#include <array>
#include <vector>
#include <queue>
#include <mutex>

namespace Alimer
{
	class D3D12CommandAllocatorPool
	{
	public:
		D3D12CommandAllocatorPool(D3D12Graphics& device, D3D12_COMMAND_LIST_TYPE type);
		~D3D12CommandAllocatorPool();

		ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);
		void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator);

		inline size_t Size() { return allocatorPool.size(); }

	private:
		D3D12Graphics& device;
		const D3D12_COMMAND_LIST_TYPE type;

		std::vector<ID3D12CommandAllocator*> allocatorPool;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> readyAllocators;
		std::mutex allocatorMutex;
	};

	class D3D12CommandQueue final : public CommandQueue
	{
		friend class D3D12CommandBuffer;

	public:
		D3D12CommandQueue(D3D12Graphics& device, CommandQueueType type);
		~D3D12CommandQueue();

		uint64_t IncrementFence();
		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFence(uint64_t fenceValue);
		void StallForFence(uint64_t fenceValue);
		void StallForProducer(D3D12CommandQueue* producer);

		CommandBuffer* GetCommandBuffer() override;
		void WaitIdle() override;
		void Submit(CommandBuffer* const* commandBuffers, uint32_t count, bool waitForCompletion) override;
		void QueuePresent(D3D12SwapChain* swapChain);

		D3D12Graphics& GetDevice() { return device; }
		ID3D12CommandQueue* GetHandle() const { return handle; }

	private:
		ID3D12CommandAllocator* RequestAllocator();
		void DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator);
		void FreeCommandBuffer(D3D12CommandBuffer* commandBuffer);

		D3D12Graphics& device;
		const CommandQueueType type;
		const D3D12_COMMAND_LIST_TYPE commandListType;
		ID3D12CommandQueue* handle;

		D3D12CommandAllocatorPool allocatorPool;
		std::mutex fenceMutex;
		std::mutex eventMutex;

		ID3D12Fence* fence = nullptr;
		HANDLE fenceEventHandle = nullptr;
		uint64_t nextFenceValue;
		uint64_t lastCompletedFenceValue;

		std::mutex cmdBuffersAllocationMutex;
		std::vector<std::unique_ptr<D3D12CommandBuffer>> cmdBuffersPool;
		std::queue<D3D12CommandBuffer*> availableCommandBuffers;

		static constexpr uint32_t kMaxSwapChains = 16u;
		uint32_t presentSwapChainsCount = 0;
		std::array<D3D12SwapChain*, kMaxSwapChains> presentSwapChains{};
	};

	constexpr D3D12CommandQueue* ToD3D12(CommandQueue* resource)
	{
		return static_cast<D3D12CommandQueue*>(resource);
	}

	inline ID3D12CommandQueue* ToD3D12Handle(CommandQueue* resource)
	{
		return static_cast<D3D12CommandQueue*>(resource)->GetHandle();
	}
}
