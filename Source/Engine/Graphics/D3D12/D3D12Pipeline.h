// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/Pipeline.h"
#include "D3D12Utils.h"
#include <array>

namespace Alimer
{
	class D3D12Pipeline final : public Pipeline
	{
	public:
		D3D12Pipeline(D3D12Graphics& device, const RenderPipelineStateCreateInfo* info);
		D3D12Pipeline(D3D12Graphics& device, const ComputePipelineCreateInfo* info);
		~D3D12Pipeline() override;
		void Destroy() override;

		ID3D12RootSignature* GetRootSignature() const { return rootSignature; }
		ID3D12PipelineState* GetHandle() const { return handle; }

		D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return primitiveTopology; }
		uint32_t GetVertexBufferSlotsUsed() const { return vboSlotsUsed;}
		uint32_t GetVertexBufferStride(uint32_t slot) const { return vboStrides[slot]; }

		uint32_t GetDescriptorCBVParameterIndex() const { return descriptorCBVParameterIndex; }
		uint32_t GetDescriptorTableRootParameterIndex() const { return descriptorTableRootParameterIndex; }
		uint32_t GetPushConstantNum32BitValues() const { return pushConstantNum32BitValues; }
		uint32_t GetDescriptorPushConstantParameterIndex() const { return descriptorPushConstantParameterIndex; }

	private:
		D3D12Graphics& device;
		ID3D12PipelineState* handle = nullptr;
		ID3D12RootSignature* rootSignature = nullptr;
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		uint32_t vboSlotsUsed = 0;
		std::array<uint32_t, kMaxVertexBufferBindings> vboStrides = {};

		uint32_t descriptorCBVParameterIndex = -1;
		uint32_t descriptorTableRootParameterIndex = -1;
		uint32_t pushConstantNum32BitValues = 0;
		uint32_t descriptorPushConstantParameterIndex = 0;
	};

	constexpr const D3D12Pipeline* ToD3D12(const Pipeline* resource)
	{
		return static_cast<const D3D12Pipeline*>(resource);
	}
}

