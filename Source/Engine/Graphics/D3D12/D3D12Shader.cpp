// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Shader.h"
#include "D3D12Graphics.h"
#include "directx/d3d12shader.h"
using Microsoft::WRL::ComPtr;

namespace Alimer
{
	D3D12Shader::D3D12Shader(D3D12Graphics& device_, ShaderStages stage_, const std::vector<uint8_t>& byteCode_, const std::string& entryPoint_)
		: Shader(stage_, byteCode_, entryPoint_)
		, device(device_)
	{
		DxcBuffer reflectionData;
		reflectionData.Encoding = DXC_CP_ACP;
		reflectionData.Ptr = byteCode.data();
		reflectionData.Size = (SIZE_T)byteCode.size();

		ComPtr<ID3D12ShaderReflection> reflection;
		ThrowIfFailed(
			device.GetDxcUtils()->CreateReflection(&reflectionData, IID_PPV_ARGS(&reflection))
		);

		D3D12_SHADER_DESC shaderDesc;
		ThrowIfFailed(reflection->GetDesc(&shaderDesc));

		for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC desc;
			ThrowIfFailed(reflection->GetResourceBindingDesc(i, &desc));

			if (desc.Type == D3D_SIT_CBUFFER)
			{
				D3D12_SHADER_BUFFER_DESC shaderBufferDesc = {};
				ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByIndex(i);
				ThrowIfFailed(constantBuffer->GetDesc(&shaderBufferDesc));

				if (desc.BindPoint >= 999)
				{
					ShaderResource resource{};
					resource.type = ShaderResourceType::PushConstant;
					resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
					resource.name = desc.Name;
					resource.offset = 0;
					resource.size = shaderBufferDesc.Size;
					resource.set = desc.Space;
					resource.binding = desc.BindPoint;
					resources.push_back(resource);
				}
				else
				{
					ShaderResource resource{};
					resource.type = ShaderResourceType::UniformBuffer;
					resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
					resource.name = desc.Name;
					resource.set = desc.Space;
					resource.binding = desc.BindPoint;
					resource.arraySize = desc.BindCount;
					resources.push_back(resource);
				}
			}
			else if (desc.Type == D3D_SIT_TEXTURE)
			{
				ShaderResource resource{};
				resource.type = ShaderResourceType::SampledTexture;
				resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
				resource.name = desc.Name;
				resource.set = desc.Space;
				resource.binding = desc.BindPoint;
				resource.arraySize = desc.BindCount;
				resources.push_back(resource);
			}
			else if (desc.Type == D3D_SIT_SAMPLER)
			{
				ShaderResource resource{};
				resource.type = ShaderResourceType::Sampler;
				resource.stages |= (ShaderStages)(1 << (uint32_t)stage);
				resource.name = desc.Name;
				resource.set = desc.Space;
				resource.binding = desc.BindPoint;
				resource.arraySize = desc.BindCount;
				resources.push_back(resource);
			}
		}

		// Update id
		std::hash<std::string> hasher{};
		hash = hasher(std::string{ byteCode.cbegin(), byteCode.cend() });
	}

	D3D12Shader::~D3D12Shader()
	{
		Destroy();
	}

	void D3D12Shader::Destroy()
	{
	}
}
