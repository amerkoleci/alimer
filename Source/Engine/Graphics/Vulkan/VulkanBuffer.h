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
		VulkanBuffer(VulkanGraphics& device, const BufferCreateInfo& info, const void* initialData);
		~VulkanBuffer() override;
		void Destroy() override;

		uint8_t* Map() override;
		void Unmap() override;

		void UploadData(const void* data, uint64_t offset, uint64_t size);
		//void Barrier(VkCommandBuffer commandBuffer, VulkanBufferState newState, 
		//	VkPipelineStageFlags srcStageMask = 0, VkPipelineStageFlags dstStageMask = 0) const;

		VkBuffer GetHandle() const { return handle; }

		//VulkanBufferState GetState() const noexcept { return state; }

	private:
		void ApiSetName() override;

		VulkanGraphics& device;
		VkBuffer handle{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };

        uint8_t* mappedData{ nullptr };

		/// Whether the buffer is persistently mapped or not
		bool persistent{ false };

		/// Whether the buffer has been mapped with vmaMapMemory
		bool mapped{ false };

		VkDeviceAddress address{ 0u };

		//mutable VulkanBufferState state{ VulkanBufferState::Undefined };
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
