// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "D3D12SwapChain.h"
#include "D3D12Graphics.h"

namespace Alimer
{
	D3D12CommandAllocatorPool::D3D12CommandAllocatorPool(D3D12Graphics& device_, D3D12_COMMAND_LIST_TYPE type_)
		: device(device_)
		, type(type_)
	{
	}

	D3D12CommandAllocatorPool::~D3D12CommandAllocatorPool()
	{
		for (size_t i = 0; i < allocatorPool.size(); ++i)
			allocatorPool[i]->Release();

		allocatorPool.clear();
	}

	ID3D12CommandAllocator* D3D12CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue)
	{
		std::lock_guard<std::mutex> LockGuard(allocatorMutex);

		ID3D12CommandAllocator* commandAllocator = nullptr;

		if (!readyAllocators.empty())
		{
			auto& allocatorPair = readyAllocators.front();

			if (allocatorPair.first <= completedFenceValue)
			{
				commandAllocator = allocatorPair.second;
				ThrowIfFailed(commandAllocator->Reset());
				readyAllocators.pop();
			}
		}

		// If no allocator's were ready to be reused, create a new one
		if (commandAllocator == nullptr)
		{
			ThrowIfFailed(device.GetHandle()->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));
#ifdef _DEBUG
			wchar_t AllocatorName[32];
			swprintf(AllocatorName, 32, L"CommandAllocator %zu", allocatorPool.size());
			commandAllocator->SetName(AllocatorName);

			LOGD("Created new CommandAllocator {}", allocatorPool.size());
#endif
			allocatorPool.push_back(commandAllocator);
		}

		return commandAllocator;
	}

	void D3D12CommandAllocatorPool::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator)
	{
		std::lock_guard<std::mutex> LockGuard(allocatorMutex);

		// That fence value indicates we are free to reset the allocator
		readyAllocators.push(std::make_pair(fenceValue, commandAllocator));
	}

	D3D12CommandQueue::D3D12CommandQueue(D3D12Graphics& device_, CommandQueueType type_)
		: CommandQueue(type_)
		, device(device_)
		, type(type_)
		, commandListType(ToD3D12(type_))
		, nextFenceValue((uint64_t)commandListType << 56 | 1)
		, lastCompletedFenceValue((uint64_t)commandListType << 56)
		, allocatorPool(device, commandListType)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = commandListType;
		queueDesc.NodeMask = 1;
		ThrowIfFailed(device.GetHandle()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&handle)));

		ThrowIfFailed(device.GetHandle()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		ThrowIfFailed(fence->Signal((uint64_t)type << 56));

		fenceEventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
		ALIMER_ASSERT(fenceEventHandle != NULL);

		switch (type)
		{
		case CommandQueueType::Graphics:
			handle->SetName(L"Graphics Command Queue");
			fence->SetName(L"Graphics Command Queue Fence");
			break;
		case CommandQueueType::Compute:
			handle->SetName(L"Compute Command Queue");
			fence->SetName(L"Compute Command Queue Fence");
			break;
		}
	}

	D3D12CommandQueue::~D3D12CommandQueue()
	{
		if (handle == nullptr)
			return;

		cmdBuffersPool.clear();

		CloseHandle(fenceEventHandle);

        fence->Release(); fence = nullptr;
        handle->Release(); handle = nullptr;
	}

	uint64_t D3D12CommandQueue::IncrementFence()
	{
		std::lock_guard<std::mutex> guard(fenceMutex);
		handle->Signal(fence, nextFenceValue);
		return nextFenceValue++;
	}

	bool D3D12CommandQueue::IsFenceComplete(uint64_t fenceValue)
	{
		// Avoid querying the fence value by testing against the last one seen.
		// The max() is to protect against an unlikely race condition that could cause the last
		// completed fence value to regress.
		if (fenceValue > lastCompletedFenceValue)
		{
			lastCompletedFenceValue = Max(lastCompletedFenceValue, fence->GetCompletedValue());
		}

		return fenceValue <= lastCompletedFenceValue;
	}

	void D3D12CommandQueue::WaitForFence(uint64_t fenceValue)
	{
		if (IsFenceComplete(fenceValue))
			return;

		{
			std::lock_guard<std::mutex> guard(eventMutex);

			fence->SetEventOnCompletion(fenceValue, fenceEventHandle);
			WaitForSingleObject(fenceEventHandle, INFINITE);
			lastCompletedFenceValue = fenceValue;
		}
	}

	void D3D12CommandQueue::StallForFence(uint64_t fenceValue)
	{
		D3D12CommandQueue* producer = device.GetD3D12CommandQueue((D3D12_COMMAND_LIST_TYPE)(fenceValue >> 56));
		handle->Wait(producer->fence, fenceValue);
	}

	void D3D12CommandQueue::StallForProducer(D3D12CommandQueue* producer)
	{
		ALIMER_ASSERT(producer->nextFenceValue > 0);
		handle->Wait(producer->fence, producer->nextFenceValue - 1);
	}

	CommandBuffer* D3D12CommandQueue::GetCommandBuffer()
	{
		std::lock_guard<std::mutex> LockGuard(cmdBuffersAllocationMutex);

		D3D12CommandBuffer* commandBuffer = nullptr;
		if (availableCommandBuffers.empty())
		{
			commandBuffer = new D3D12CommandBuffer(*this);
			cmdBuffersPool.emplace_back(commandBuffer);
		}
		else
		{
			commandBuffer = availableCommandBuffers.front();
			availableCommandBuffers.pop();
			commandBuffer->Reset(device.GetFrameIndex());
		}

		ALIMER_ASSERT(commandBuffer != nullptr);

		return commandBuffer;
	}

	void D3D12CommandQueue::FreeCommandBuffer(D3D12CommandBuffer* commandBuffer)
	{
		ALIMER_ASSERT(commandBuffer != nullptr);

		std::lock_guard<std::mutex> LockGuard(cmdBuffersAllocationMutex);
		availableCommandBuffers.push(commandBuffer);
	}

	void D3D12CommandQueue::WaitIdle()
	{
		WaitForFence(IncrementFence());
	}

	void D3D12CommandQueue::Submit(CommandBuffer* const* commandBuffers, uint32_t count, bool waitForCompletion)
	{
		device.SyncCopyQueue();

		std::lock_guard<std::mutex> guard(fenceMutex);
		std::vector<ID3D12CommandList*> d3d12CommandLists;
		const uint64_t fenceValue = nextFenceValue;

		for (uint32_t i = 0; i < count; i++)
		{
			D3D12CommandBuffer* d3d12CommandBuffer = ToD3D12(commandBuffers[i]);
			d3d12CommandLists.push_back(d3d12CommandBuffer->FinishAndReturn(fenceValue));

			FreeCommandBuffer(d3d12CommandBuffer);
		}

		UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
		handle->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());

		// Signal the next fence value (with the GPU) and increment the fence value.  
		handle->Signal(fence, nextFenceValue++);

		if (waitForCompletion)
		{
			WaitForFence(fenceValue);
		}

		// Handle automatic SwapChain queue present
		if (queueType == CommandQueueType::Graphics)
		{
			if (presentSwapChainsCount > 0)
			{
				for (size_t i = 0; i < presentSwapChainsCount; i++)
				{
					HRESULT hr = presentSwapChains[i]->Present();

					// If the device was reset we must completely reinitialize the renderer.
					if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
					{
						device.HandleDeviceLost(hr);
					}
				}
			}
		}

		presentSwapChainsCount = 0;
	}

	void D3D12CommandQueue::QueuePresent(D3D12SwapChain* swapChain)
	{
		ALIMER_ASSERT(queueType == CommandQueueType::Graphics);
		presentSwapChains[presentSwapChainsCount++] = swapChain;
	}

	ID3D12CommandAllocator* D3D12CommandQueue::RequestAllocator()
	{
		uint64_t completedFenceValue = fence->GetCompletedValue();
		return allocatorPool.RequestAllocator(completedFenceValue);
	}

	void D3D12CommandQueue::DiscardAllocator(uint64_t fenceValue, ID3D12CommandAllocator* commandAllocator)
	{
		allocatorPool.DiscardAllocator(fenceValue, commandAllocator);
	}
}
