// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanCommandQueue.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphics.h"

namespace Alimer
{
	VulkanCommandQueue::VulkanCommandQueue(VulkanGraphics& device_, CommandQueueType type, uint32_t queueFamilyIndex, uint32_t queueIndex)
		: CommandQueue(type)
		, device(device_)
	{
		vkGetDeviceQueue(device.GetHandle(), queueFamilyIndex, queueIndex, &handle);

		VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndex;

		for (uint32_t i = 0; i < kMaxFramesInFlight; i++)
		{
			VK_CHECK(vkCreateCommandPool(device.GetHandle(), &poolInfo, nullptr, &commandBuffers[i].commandPool));
		}
	}

	VulkanCommandQueue::~VulkanCommandQueue()
	{
		for (uint32_t i = 0; i < kMaxFramesInFlight; i++)
		{
			for (uint32_t j = 0; j < kMaxCommandLists; j++)
			{
				commandBuffers[i].commandBuffers[j].reset();
			}

			vkDestroyCommandPool(device.GetHandle(), commandBuffers[i].commandPool, nullptr);
		}
	}

	void VulkanCommandQueue::Reset(uint32_t frameIndex_)
	{
		frameIndex = frameIndex_;

		commandBuffers[frameIndex].allocatedBuffers = 0;
		VK_CHECK(vkResetCommandPool(device.GetHandle(), commandBuffers[frameIndex].commandPool, 0));
	}

	void VulkanCommandQueue::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags waitDstStageMask)
	{
		ALIMER_ASSERT(semaphore != VK_NULL_HANDLE);

		waitSemaphores.push_back(semaphore);
		waitDstStageMasks.push_back(waitDstStageMask);
        waitValues.push_back(0);
	}

	void VulkanCommandQueue::AddSignalSemaphore(VkSemaphore semaphore)
	{
		ALIMER_ASSERT(semaphore != VK_NULL_HANDLE);

		signalSemaphores.push_back(semaphore);
	}

	void VulkanCommandQueue::QueuePresent(VulkanSwapChain* swapChain)
	{
		ALIMER_ASSERT(queueType == CommandQueueType::Graphics);

		presentSwapChains[presentSwapChainsCount] = swapChain;
		vkPresentSwapChains[presentSwapChainsCount] = swapChain->GetHandle();
		imageIndices[presentSwapChainsCount] = swapChain->GetBackBufferIndex();
		presentSwapChainsCount++;
	}

	CommandBuffer* VulkanCommandQueue::GetCommandBuffer()
	{
		auto& frameCmd = commandBuffers[frameIndex];

		if (frameCmd.allocatedBuffers >= kMaxCommandLists)
		{
            LOGE("Vulkan: Max command buffer count reached!");
			return nullptr;
		}

		if (frameCmd.commandBuffers[frameCmd.allocatedBuffers] == nullptr)
		{
			frameCmd.commandBuffers[frameCmd.allocatedBuffers].reset(new VulkanCommandBuffer(*this, frameCmd.commandPool));
		}

		// Begin recording
		frameCmd.commandBuffers[frameCmd.allocatedBuffers]->Reset(frameIndex);

		return frameCmd.commandBuffers[frameCmd.allocatedBuffers++].get();
	}

	void VulkanCommandQueue::WaitIdle()
	{
		VK_CHECK(vkQueueWaitIdle(handle));
	}

	void VulkanCommandQueue::Submit(CommandBuffer* const* commandBuffers, uint32_t count, bool waitForCompletion)
	{
        const uint64_t copyFenceValue = device.FlushCopy();

        // Sync with copy queue
        if (copyFenceValue > 0)
        {
            waitDstStageMasks.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
            waitSemaphores.push_back(device.GetCopySemaphore());
            waitValues.push_back(copyFenceValue);
        }

		std::vector<VkCommandBuffer> vkCommandBuffers;

		for (uint32_t i = 0; i < count; i++)
		{
			VulkanCommandBuffer* vulkanCommandBuffer = ToVulkan(commandBuffers[i]);
			vulkanCommandBuffer->End();
			vkCommandBuffers.push_back(vulkanCommandBuffer->GetHandle());
		}

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitDstStageMasks.data();
		submitInfo.commandBufferCount = count;
		submitInfo.pCommandBuffers = vkCommandBuffers.data();
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();

        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo{ VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
        timelineSubmitInfo.pNext = nullptr;
        if (waitValues.size())
        {
            timelineSubmitInfo.waitSemaphoreValueCount = (uint32_t)waitValues.size();
            timelineSubmitInfo.pWaitSemaphoreValues = waitValues.data();
            timelineSubmitInfo.signalSemaphoreValueCount = (uint32_t)signalValues.size();
            timelineSubmitInfo.pSignalSemaphoreValues = signalValues.data();

            submitInfo.pNext = &timelineSubmitInfo;
        }

		VkFence fence = device.AcquireFence();

		VkResult result = vkQueueSubmit(handle, 1, &submitInfo, fence);
		if (result != VK_SUCCESS)
		{
            LOGE("Vulkan: Failed to submit command buffer!");
			return;
		}

		if (waitForCompletion)
		{
			vkWaitForFences(device.GetHandle(), 1, &fence, VK_TRUE, 100000000000);
			device.ReleaseFence(fence);
		}
		else
		{
			device.SubmitFence(fence);
		}

		// Handle automatic SwapChain queue present
		if (queueType == CommandQueueType::Graphics)
		{
			if (presentSwapChainsCount > 0)
			{
				VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
				presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
				presentInfo.pWaitSemaphores = signalSemaphores.data();
				presentInfo.swapchainCount = presentSwapChainsCount;
				presentInfo.pSwapchains = vkPresentSwapChains.data();
				presentInfo.pImageIndices = imageIndices.data();
				presentInfo.pResults = presentResults.data();

				vkQueuePresentKHR(handle, &presentInfo);

				for (size_t i = 0; i < presentSwapChainsCount; i++)
				{
					presentSwapChains[i]->AfterPresent(presentResults[i]);
				}
			}
		}

        // Clear wait and signal data
        waitSemaphores.clear();
        waitDstStageMasks.clear();
        signalSemaphores.clear();
        waitValues.clear();
        signalValues.clear();
        presentSwapChainsCount = 0;
	}
}
