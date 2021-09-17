// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Shader.h"
#include "VulkanUtils.h"

namespace Alimer
{
	class VulkanShader final : public Shader
	{
	public:
		VulkanShader(VulkanGraphics& device, ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint);
		~VulkanShader();
		void Destroy() override;

		VkShaderModule GetHandle() const { return handle; }

	private:
		VulkanGraphics& device;
		VkShaderModule handle = VK_NULL_HANDLE;
	};

	constexpr VulkanShader* ToVulkan(Shader* resource)
	{
		return static_cast<VulkanShader*>(resource);
	}
}

