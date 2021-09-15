// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Sampler.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanSampler final : public Sampler
	{
	public:
		VulkanSampler(VulkanGraphics& device, const SamplerCreateInfo* info);
		~VulkanSampler() override;
		void Destroy() override;

		VkSampler GetHandle() const { return handle; }

	private:
        void ApiSetName() override;

		VulkanGraphics& device;
		VkSampler handle{ VK_NULL_HANDLE };
	};

	constexpr VulkanSampler* ToVulkan(Sampler* resource)
	{
		return static_cast<VulkanSampler*>(resource);
	}

	constexpr const VulkanSampler* ToVulkan(const Sampler* resource)
	{
		return static_cast<const VulkanSampler*>(resource);
	}
}
