// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Texture.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanSwapChain;

	class VulkanTexture final : public Texture
	{
	public:
		VulkanTexture(VulkanGraphics& device, const TextureCreateInfo& info, VkImage existingImage = VK_NULL_HANDLE, const void* initialData = nullptr);
		~VulkanTexture() override;
		void Destroy() override;

		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        TextureView* CreateView(const TextureViewCreateInfo& createInfo) override;

        VulkanGraphics& GetDevice() { return device; }
		VkImage GetHandle() const { return handle; }
		VkImageLayout GetLayout() const noexcept { return layout; }
		void SetLayout(VkImageLayout newLayout) const noexcept { layout = newLayout; }

		// Swapchain texture logic.
		bool IsSwapChainTexture() const { return swapChain != nullptr; }
		VulkanSwapChain* GetSwapChain() const { return swapChain; }
		void SetSwapChain(VulkanSwapChain* swapChain_) { swapChain = swapChain_; }
		VkSemaphore GetWaitSemaphore() const { return waitSemaphore; }
		void SetWaitSemaphore(VkSemaphore semaphore) { waitSemaphore = semaphore; }
		VkSemaphore GetSignalSemaphore() const { return signalSemaphore; }
		void SetSignalSemaphore(VkSemaphore semaphore) { signalSemaphore = semaphore; }

	private:
        void ApiSetName() override;

		VulkanGraphics& device;
		VkImage handle{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		mutable VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };

		VulkanSwapChain* swapChain = nullptr;
		VkSemaphore waitSemaphore = VK_NULL_HANDLE;
		VkSemaphore signalSemaphore = VK_NULL_HANDLE;
	};

    class VulkanTextureView final : public TextureView
    {
    private:
        VulkanGraphics& device;
        VkImageView handle{ VK_NULL_HANDLE };

    public:
        VulkanTextureView(_In_ VulkanTexture* texture, const TextureViewCreateInfo& info);
        ~VulkanTextureView() override;

        VkImageView GetHandle() const { return handle; }
    };

	constexpr const VulkanTexture* ToVulkan(const Texture* resource)
	{
		return static_cast<const VulkanTexture*>(resource);
	}

    constexpr const VulkanTextureView* ToVulkan(const TextureView* resource)
    {
        return static_cast<const VulkanTextureView*>(resource);
    }
}

