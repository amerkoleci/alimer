// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/SwapChain.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanSwapChain final : public SwapChain
	{
	public:
		VulkanSwapChain(VulkanGraphics& device, void* windowHandle, const SwapChainCreateInfo& info);
		~VulkanSwapChain() override;
		void Destroy() override;

		void AfterPresent(VkResult result);
		VkSwapchainKHR GetHandle() const { return handle; }
		uint32_t GetBackBufferIndex() const noexcept { return backBufferIndex; }
		TextureView* GetCurrentTextureView() const override;

	private:
		void ResizeBackBuffer(uint32_t width, uint32_t height) override;
		void Destroy(bool destroyHandle);
		void Recreate();

		VkResult AcquireNextImage() const;
		VulkanGraphics& device;

		VkSurfaceKHR surface;
		VkSwapchainKHR handle = VK_NULL_HANDLE;

		uint32_t bufferCount = kMaxFramesInFlight;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderCompleteSemaphores;
		std::vector<VkFence> imageAcquiredFences;
		mutable std::vector<bool> imageAcquiredFenceSubmitted;
		uint32_t semaphoreIndex = 0;

		bool isMinimized = false;

		mutable bool needAcquire = true;
		mutable uint32_t backBufferIndex = 0;
		std::vector<RefPtr<VulkanTexture>> backBufferTextures;
	};
}

