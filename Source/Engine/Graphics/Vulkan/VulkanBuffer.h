// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Buffer.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanBuffer final : public Buffer
	{
	public:
		VulkanBuffer(VulkanGraphics& device, const BufferCreateInfo* info, const void* initialData);
		~VulkanBuffer() override;
		void Destroy() override;

		VkBuffer GetHandle() const { return handle; }

	private:
		void ApiSetName() override;

		VulkanGraphics& device;
		VkBuffer handle{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
	};

	constexpr VulkanBuffer* ToVulkan(Buffer* resource)
	{
		return static_cast<VulkanBuffer*>(resource);
	}

	constexpr const VulkanBuffer* ToVulkan(const Buffer* resource)
	{
		return static_cast<const VulkanBuffer*>(resource);
	}
}
