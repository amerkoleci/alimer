// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanShader.h"
#include "VulkanGraphics.h"
#include <spirv_reflect.h>

namespace Alimer
{
	namespace
	{
		[[nodiscard]] constexpr ShaderStages ConvertShaderStage(SpvExecutionModel model)
		{
			switch (model)
			{
			case SpvExecutionModelVertex:
				return ShaderStages::Vertex;
				//case SpvExecutionModelTessellationControl:
				//	return ShaderStages::TessControl;
				//case SpvExecutionModelTessellationEvaluation:
				//	return ShaderStages::TessEvaluation;
				//case SpvExecutionModelGeometry:
				//	return ShaderStages::Geometry;
			case SpvExecutionModelFragment:
				return ShaderStages::Pixel;
			case SpvExecutionModelGLCompute:
				return ShaderStages::Compute;
				//case SpvExecutionModelRayGenerationNV:
				//	return ShaderStage::RayGeneration;
				//case SpvExecutionModelIntersectionNV:
				//	return ShaderStage::Intersection;
				//case SpvExecutionModelAnyHitNV:
				//	return ShaderStage::AnyHit;
				//case SpvExecutionModelClosestHitNV:
				//	return ShaderStage::ClosestHit;
				//case SpvExecutionModelMissNV:
				//	return ShaderStage::Miss;
				//case SpvExecutionModelCallableNV:
				//	return ShaderStage::Callable;
				//case SpvExecutionModelTaskNV:
				//	return ShaderStage::Amplification;
				//case SpvExecutionModelMeshNV:
				//	return ShaderStage::Mesh;
			}

			ALIMER_UNREACHABLE();
			return ShaderStages::None;
		}

		[[nodiscard]] constexpr ShaderResourceType GetShaderResourceType(SpvReflectDescriptorType type)
		{
			switch (type)
			{
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
				return ShaderResourceType::Sampler;
			case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				return ShaderResourceType::SampledTexture;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				return ShaderResourceType::StorageTexture;

			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				return ShaderResourceType::UniformBuffer;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
				return ShaderResourceType::StorageBuffer;

			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				return ShaderResourceType::UniformBuffer;

			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				break;
			case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
				break;
			default:
				ALIMER_UNREACHABLE();
				break;
			}

			ALIMER_UNREACHABLE();
		}
	}

	VulkanShader::VulkanShader(VulkanGraphics& device_, ShaderStages stage_, const std::vector<uint8_t>& byteCode_, const std::string& entryPoint_)
		: Shader(stage_, byteCode_, entryPoint_)
		, device(device_)
	{
		SpvReflectShaderModule module;
		SpvReflectResult result = spvReflectCreateShaderModule(byteCode_.size(), byteCode_.data(), &module);
		ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

		const SpvReflectEntryPoint* spvEntryPoint = spvReflectGetEntryPoint(&module, entryPoint.c_str());
		ALIMER_ASSERT(spvEntryPoint != nullptr);
		ALIMER_ASSERT(module.entry_point_count == 1);

		stage = ConvertShaderStage(spvEntryPoint->spirv_execution_model);

		// Bindings
		{
			uint32_t count = 0;
			result = spvReflectEnumerateEntryPointDescriptorBindings(&module, entryPoint.c_str(), &count, NULL);
			ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectDescriptorBinding*> bindings(count);
			result = spvReflectEnumerateEntryPointDescriptorBindings(&module, entryPoint.c_str(), &count, bindings.data());
			ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			const uint32_t bindlessSet = 1;
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			for (auto& binding : bindings)
			{
				const bool bindless = binding->set > 0;

				ShaderResource resource{};
				resource.type = GetShaderResourceType(binding->descriptor_type);
				resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
				resource.name = binding->name;

				resource.arraySize = binding->count;
				for (uint32_t i_dim = 0; i_dim < binding->array.dims_count; ++i_dim) {
					resource.arraySize *= binding->array.dims[i_dim];
				}

				if (binding->set == 0)
				{
					switch (binding->resource_type)
					{
					case SPV_REFLECT_RESOURCE_FLAG_SAMPLER:
						spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_Sampler, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
						break;

					case SPV_REFLECT_RESOURCE_FLAG_CBV: // Unchanged
						spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_CBV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
						break;

					case SPV_REFLECT_RESOURCE_FLAG_SRV:
						spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_SRV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
						break;
					case SPV_REFLECT_RESOURCE_FLAG_UAV:
						spvReflectChangeDescriptorBindingNumbers(&module, binding, binding->binding + kVulkanBindingShift_UAV, SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
						break;
					}
				}

				resource.set = binding->set;
				resource.binding = binding->binding;
				resources.push_back(resource);
			}
		}

		// Push constants
		{
			uint32_t pushCount = 0;
			result = spvReflectEnumerateEntryPointPushConstantBlocks(&module, entryPoint.c_str(), &pushCount, nullptr);
			ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectBlockVariable*> pushConstants(pushCount);
			result = spvReflectEnumerateEntryPointPushConstantBlocks(&module, entryPoint.c_str(), &pushCount, pushConstants.data());
			ALIMER_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

			for (auto& pushConstant : pushConstants)
			{
				ShaderResource resource{};
				resource.type = ShaderResourceType::PushConstant;
				resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
				resource.name = pushConstant->name;
				resource.offset = pushConstant->offset;
				resource.size = pushConstant->size;
				resources.push_back(resource);
			}
		}

		byteCode.clear();
		byteCode.resize(spvReflectGetCodeSize(&module));
		memcpy(byteCode.data(), spvReflectGetCode(&module), byteCode.size());
		spvReflectDestroyShaderModule(&module);

		// Create the Vulkan ShaderModule.
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = byteCode.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());
			VkResult result = vkCreateShaderModule(device.GetHandle(), &createInfo, nullptr, &handle);

			if (result != VK_SUCCESS)
			{
				VK_LOG_ERROR(result, "Failed to create ShaderModule");
				return;
			}
		}

		std::hash<std::string> hasher{};
		hash = hasher(std::string{ byteCode.cbegin(), byteCode.cend() });

        OnCreated();
	}

	VulkanShader::~VulkanShader()
	{
		Destroy();
	}

	void VulkanShader::Destroy()
	{
		if (handle != VK_NULL_HANDLE)
		{
			device.DeferDestroy(handle);
			handle = VK_NULL_HANDLE;
		}

        OnDestroyed();
	}
}

