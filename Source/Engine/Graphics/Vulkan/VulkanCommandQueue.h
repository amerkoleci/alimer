// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/CommandQueue.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanSwapChain;

	class VulkanCommandQueue final : public CommandQueue
	{
	public:
		VulkanCommandQueue(VulkanGraphics& device, QueueType type, uint32_t queueFamilyIndex, uint32_t queueIndex = 0);
		~VulkanCommandQueue() override;

		void Reset(uint32_t frameIndex_);
		void AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitDstStageMask);
		void AddSignalSemaphore(VkSemaphore semaphore);
		void QueuePresent(VulkanSwapChain* swapChain);

		CommandBuffer* GetCommandBuffer() override;
		void WaitIdle() override;
		void Submit(CommandBuffer* const* commandBuffers, uint32_t count, bool waitForCompletion) override;

		VulkanGraphics& GetDevice() { return device; }
		VkQueue GetHandle() const { return handle; }
		VkCommandPool GetCommandPool() const { return commandBuffers[frameIndex].commandPool; }

	private:
		VulkanGraphics& device;
		VkQueue handle = VK_NULL_HANDLE;
		uint32_t frameIndex = 0;

		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkPipelineStageFlags> waitDstStageMasks;
		std::vector<VkSemaphore> signalSemaphores;

        std::vector<uint64_t> waitValues;
        std::vector<uint64_t> signalValues;

		static constexpr uint32_t kMaxSwapChains = 16u;
		uint32_t presentSwapChainsCount = 0;

		std::array<VulkanSwapChain*, kMaxSwapChains> presentSwapChains{};
		std::array<VkSwapchainKHR, kMaxSwapChains> vkPresentSwapChains{};
		std::array<uint32_t, kMaxSwapChains> imageIndices{};
        std::array<VkResult, kMaxSwapChains> presentResults{};
        
		struct FrameCommandBuffers
		{
			VkCommandPool commandPool = VK_NULL_HANDLE;
			uint32_t allocatedBuffers = 0;
			std::unique_ptr<VulkanCommandBuffer> commandBuffers[kMaxCommandLists] = {};
		} commandBuffers[kMaxFramesInFlight] = {};
	};

	constexpr VulkanCommandQueue* ToVulkan(CommandQueue* resource)
	{
		return static_cast<VulkanCommandQueue*>(resource);
	}

	constexpr const VulkanCommandQueue* ToVulkan(const CommandQueue* resource)
	{
		return static_cast<const VulkanCommandQueue*>(resource);
	}
}

