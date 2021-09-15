// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "D3D12Pipeline.h"
#include "D3D12Shader.h"
#include "D3D12Graphics.h"
#include "directx/d3dx12.h"

namespace Alimer
{
	namespace
	{
		[[nodiscard]] constexpr D3D_PRIMITIVE_TOPOLOGY ToD3D12PrimitiveTopology(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::PointList:
				return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList:
				return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleList:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::TriangleStrip:
				return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			default:
				ALIMER_UNREACHABLE();
				return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			}
		}

		[[nodiscard]] constexpr D3D12_INPUT_CLASSIFICATION ToD3D12(VertexStepRate stepMode)
		{
			switch (stepMode)
			{
			case VertexStepRate::Instance:
				return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			default:
				return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			}
		}

		[[nodiscard]] constexpr DXGI_FORMAT ToD3D12(VertexFormat format)
		{
			switch (format)
			{
			//case VertexFormat::UChar2:
			//	return DXGI_FORMAT_R8G8_UINT;
			//case VertexFormat::UChar4:
			//	return DXGI_FORMAT_R8G8B8A8_UINT;
			//case VertexFormat::Char2:
			//	return DXGI_FORMAT_R8G8_SINT;
			//case VertexFormat::Char4:
			//	return DXGI_FORMAT_R8G8B8A8_SINT;
			//case VertexFormat::UChar2Norm:
			//	return DXGI_FORMAT_R8G8_UNORM;
			//case VertexFormat::UChar4Norm:
			//	return DXGI_FORMAT_R8G8B8A8_UNORM;
			//case VertexFormat::Char2Norm:
			//	return DXGI_FORMAT_R8G8_SNORM;
			//case VertexFormat::Char4Norm:
			//	return DXGI_FORMAT_R8G8B8A8_SNORM;
			case VertexFormat::UShort2:
				return DXGI_FORMAT_R16G16_UINT;
			case VertexFormat::UShort4:
				return DXGI_FORMAT_R16G16B16A16_UINT;
			case VertexFormat::Short2:
				return DXGI_FORMAT_R16G16_SINT;
			case VertexFormat::Short4:
				return DXGI_FORMAT_R16G16B16A16_SINT;
			case VertexFormat::UShort2Norm:
				return DXGI_FORMAT_R16G16_UNORM;
			case VertexFormat::UShort4Norm:
				return DXGI_FORMAT_R16G16B16A16_UNORM;
			case VertexFormat::Short2Norm:
				return DXGI_FORMAT_R16G16_SNORM;
			case VertexFormat::Short4Norm:
				return DXGI_FORMAT_R16G16B16A16_SNORM;
			case VertexFormat::Half2:
				return DXGI_FORMAT_R16G16_FLOAT;
			case VertexFormat::Half4:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case VertexFormat::Float:
				return DXGI_FORMAT_R32_FLOAT;
			case VertexFormat::Float2:
				return DXGI_FORMAT_R32G32_FLOAT;
			case VertexFormat::Float3:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			case VertexFormat::Float4:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case VertexFormat::UInt:
				return DXGI_FORMAT_R32_UINT;
			case VertexFormat::UInt2:
				return DXGI_FORMAT_R32G32_UINT;
			case VertexFormat::UInt3:
				return DXGI_FORMAT_R32G32B32_UINT;
			case VertexFormat::UInt4:
				return DXGI_FORMAT_R32G32B32A32_UINT;

			case VertexFormat::Int:
				return DXGI_FORMAT_R32_SINT;
			case VertexFormat::Int2:
				return DXGI_FORMAT_R32G32_SINT;
			case VertexFormat::Int3:
				return DXGI_FORMAT_R32G32B32_SINT;
			case VertexFormat::Int4:
				return DXGI_FORMAT_R32G32B32A32_SINT;

			case VertexFormat::RGB10A2Unorm:
				return DXGI_FORMAT_R10G10B10A2_UNORM;

			default:
				ALIMER_UNREACHABLE();
			}
		}

		[[nodiscard]] constexpr D3D12_FILL_MODE ToD3D12(FillMode mode)
		{
			switch (mode)
			{
			default:
			case FillMode::Solid:
				return D3D12_FILL_MODE_SOLID;
			case FillMode::Wireframe:
				return D3D12_FILL_MODE_WIREFRAME;
			}
		}

		[[nodiscard]] constexpr D3D12_CULL_MODE ToD3D12(CullMode mode)
		{
			switch (mode)
			{
			default:
			case CullMode::None:
				return D3D12_CULL_MODE_NONE;
			case CullMode::Front:
				return D3D12_CULL_MODE_FRONT;
			case CullMode::Back:
				return D3D12_CULL_MODE_BACK;
			}
		}

		[[nodiscard]] constexpr D3D12_BLEND D3D12Blend(BlendFactor factor)
		{
			switch (factor) {
			case BlendFactor::Zero:
				return D3D12_BLEND_ZERO;
			case BlendFactor::One:
				return D3D12_BLEND_ONE;
			case BlendFactor::SourceColor:
				return D3D12_BLEND_SRC_COLOR;
			case BlendFactor::OneMinusSourceColor:
				return D3D12_BLEND_INV_SRC_COLOR;
			case BlendFactor::SourceAlpha:
				return D3D12_BLEND_SRC_ALPHA;
			case BlendFactor::OneMinusSourceAlpha:
				return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFactor::DestinationColor:
				return D3D12_BLEND_DEST_COLOR;
			case BlendFactor::OneMinusDestinationColor:
				return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFactor::DestinationAlpha:
				return D3D12_BLEND_DEST_ALPHA;
			case BlendFactor::OneMinusDestinationAlpha:
				return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFactor::SourceAlphaSaturated:
				return D3D12_BLEND_SRC_ALPHA_SAT;
			case BlendFactor::BlendColor:
				return D3D12_BLEND_BLEND_FACTOR;
			case BlendFactor::OneMinusBlendColor:
				return D3D12_BLEND_INV_BLEND_FACTOR;
			case BlendFactor::Source1Color:
				return D3D12_BLEND_SRC1_COLOR;
			case BlendFactor::OneMinusSource1Color:
				return D3D12_BLEND_INV_SRC1_COLOR;
			case BlendFactor::Source1Alpha:
				return D3D12_BLEND_SRC1_ALPHA;
			case BlendFactor::OneMinusSource1Alpha:
				return D3D12_BLEND_INV_SRC1_ALPHA;
			default:
				ALIMER_UNREACHABLE();
			}
		}

        [[nodiscard]] constexpr D3D12_BLEND D3D12AlphaBlend(BlendFactor factor)
        {
            switch (factor) {
                case BlendFactor::SourceColor:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::OneMinusSourceColor:
                    return D3D12_BLEND_INV_SRC_ALPHA;
                case BlendFactor::DestinationColor:
                    return D3D12_BLEND_DEST_ALPHA;
                case BlendFactor::OneMinusDestinationColor:
                    return D3D12_BLEND_INV_DEST_ALPHA;
                case BlendFactor::SourceAlpha:
                    return D3D12_BLEND_SRC_ALPHA;
                case BlendFactor::Source1Color:
                    return D3D12_BLEND_SRC1_ALPHA;
                case BlendFactor::OneMinusSource1Color:
                    return D3D12_BLEND_INV_SRC1_ALPHA;
                    // Other blend factors translate to the same D3D12 enum as the color blend factors.
                default:
                    return D3D12Blend(factor);
            }
        }

		[[nodiscard]] constexpr D3D12_BLEND_OP D3D12BlendOperation(BlendOperation operation)
		{
			switch (operation)
            {
			case BlendOperation::Add:
				return D3D12_BLEND_OP_ADD;
			case BlendOperation::Subtract:
				return D3D12_BLEND_OP_SUBTRACT;
			case BlendOperation::ReverseSubtract:
				return D3D12_BLEND_OP_REV_SUBTRACT;
			case BlendOperation::Min:
				return D3D12_BLEND_OP_MIN;
			case BlendOperation::Max:
				return D3D12_BLEND_OP_MAX;
			default:
				ALIMER_UNREACHABLE();
			}
		}

		[[nodiscard]] constexpr uint8_t D3D12RenderTargetWriteMask(ColorWriteMask mask)
		{
			static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Red) == D3D12_COLOR_WRITE_ENABLE_RED, "ColorWriteMask mismatch");
			static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Green) == D3D12_COLOR_WRITE_ENABLE_GREEN, "ColorWriteMask mismatch");
			static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Blue) == D3D12_COLOR_WRITE_ENABLE_BLUE, "ColorWriteMask mismatch");
			static_assert(static_cast<D3D12_COLOR_WRITE_ENABLE>(ColorWriteMask::Alpha) == D3D12_COLOR_WRITE_ENABLE_ALPHA, "ColorWriteMask mismatch");
			return static_cast<uint8_t>(mask);
		}

		[[nodiscard]] constexpr D3D12_STENCIL_OP ToD3D12(StencilOperation op)
		{
			switch (op) {
			case StencilOperation::Keep:
				return D3D12_STENCIL_OP_KEEP;
			case StencilOperation::Zero:
				return D3D12_STENCIL_OP_ZERO;
			case StencilOperation::Replace:
				return D3D12_STENCIL_OP_REPLACE;
			case StencilOperation::IncrementClamp:
				return D3D12_STENCIL_OP_INCR_SAT;
			case StencilOperation::DecrementClamp:
				return D3D12_STENCIL_OP_DECR_SAT;
			case StencilOperation::Invert:
				return D3D12_STENCIL_OP_INVERT;
			case StencilOperation::IncrementWrap:
				return D3D12_STENCIL_OP_INCR;
			case StencilOperation::DecrementWrap:
				return D3D12_STENCIL_OP_DECR;
			default:
				ALIMER_UNREACHABLE();
			}
		}
	}

	D3D12Pipeline::D3D12Pipeline(D3D12Graphics& device_, const RenderPipelineStateCreateInfo* info)
		: Pipeline(Type::RenderPipeline)
		, device(device_)
	{
		std::unordered_map<std::string, ShaderResource> shaderResources;
		std::vector<Shader*> shaders;
		shaders.push_back(info->vertexShader);
		shaders.push_back(info->fragmentShader);

		for (auto* shader : shaders)
		{
			for (const auto& resource : shader->GetResources())
			{
				std::string key = resource.name;

				// Since 'Input' and 'Output' resources can have the same name, we modify the key string
				if (resource.type == ShaderResourceType::Input ||
					resource.type == ShaderResourceType::Output)
				{
					//key = std::to_string(resource.stages) + "_" + key;
				}

				auto it = shaderResources.find(key);

				if (it != shaderResources.end())
				{
					// Append stage flags if resource already exists
					it->second.stages |= resource.stages;
				}
				else
				{
					// Create a new entry in the map
					shaderResources.emplace(key, resource);
				}
			}
		}

		std::vector<D3D12_DESCRIPTOR_RANGE1> descriptorRanges;
		std::vector<D3D12_ROOT_PARAMETER1> rootParameters;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;

		for (auto& it : shaderResources)
		{
			ShaderResource& shaderResource = it.second;

			if (shaderResource.type == ShaderResourceType::Input ||
				shaderResource.type == ShaderResourceType::Output)
			{
				continue;
			}

			if (shaderResource.type == ShaderResourceType::UniformBuffer)
			{
				descriptorCBVParameterIndex = (uint32_t)rootParameters.size();
				D3D12_ROOT_PARAMETER1& rootParameter = rootParameters.emplace_back();
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				rootParameter.Descriptor.ShaderRegister = shaderResource.binding;
				rootParameter.Descriptor.RegisterSpace = shaderResource.set;
			}
			else if (shaderResource.type == ShaderResourceType::PushConstant)
			{
				descriptorPushConstantParameterIndex = (uint32_t)rootParameters.size();
				D3D12_ROOT_PARAMETER1& rootParameter = rootParameters.emplace_back();
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				rootParameter.Constants.ShaderRegister = shaderResource.binding;
				rootParameter.Constants.RegisterSpace = shaderResource.set;
				rootParameter.Constants.Num32BitValues = shaderResource.size / sizeof(uint32_t);
				pushConstantNum32BitValues = rootParameter.Constants.Num32BitValues;
			}
			else if (shaderResource.type == ShaderResourceType::SampledTexture)
			{
				D3D12_DESCRIPTOR_RANGE1& descriptorRange = descriptorRanges.emplace_back();
				descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				descriptorRange.NumDescriptors = shaderResource.arraySize;
				descriptorRange.BaseShaderRegister = shaderResource.binding;
				descriptorRange.RegisterSpace = shaderResource.set;
				descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				if (shaderResource.arraySize == 0) // bindless
				{
					descriptorRange.NumDescriptors = UINT_MAX;
					descriptorRange.OffsetInDescriptorsFromTableStart = 0;
				}
				descriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
			}
			else if (shaderResource.type == ShaderResourceType::Sampler)
			{
				D3D12_STATIC_SAMPLER_DESC& staticSampler = staticSamplers.emplace_back();
				staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				staticSampler.MipLODBias = 0.0f;
				staticSampler.MaxAnisotropy = 1;
				staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				staticSampler.MinLOD = 0;
				staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
				staticSampler.ShaderRegister = 0;
				staticSampler.RegisterSpace = 0;
				staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
		}

		if (descriptorRanges.size() > 0)
		{
			descriptorTableRootParameterIndex = (uint32_t)rootParameters.size();
			D3D12_ROOT_PARAMETER1& rootParameter = rootParameters.emplace_back();
			rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			rootParameter.DescriptorTable.NumDescriptorRanges = (UINT)descriptorRanges.size();
			rootParameter.DescriptorTable.pDescriptorRanges = descriptorRanges.data();
		}

		D3D12_ROOT_SIGNATURE_DESC1 rootSigDesc = {};
		rootSigDesc.NumParameters = (UINT)rootParameters.size();
		rootSigDesc.pParameters = rootParameters.data();
		rootSigDesc.NumStaticSamplers = (UINT)staticSamplers.size();
		rootSigDesc.pStaticSamplers = staticSamplers.data();
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSignature = device.CreateRootSignature(rootSigDesc);

		D3D12_RT_FORMAT_ARRAY RTVFormats{};

		struct PSO_STREAM
		{
			struct PSO_STREAM1
			{
				CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
				CD3DX12_PIPELINE_STATE_STREAM_VS VS;
				//CD3DX12_PIPELINE_STATE_STREAM_HS HS;
				//CD3DX12_PIPELINE_STATE_STREAM_DS DS;
				//CD3DX12_PIPELINE_STATE_STREAM_GS GS;
				CD3DX12_PIPELINE_STATE_STREAM_PS PS;
				CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
				CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK SampleMask;
				CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
				CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
				CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE IBStripCutValue;
				CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
				CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
				CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
				CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
			} stream1 = {};

			struct PSO_STREAM2
			{
				CD3DX12_PIPELINE_STATE_STREAM_MS MS;
				CD3DX12_PIPELINE_STATE_STREAM_AS AS;
			} stream2 = {};
		} stream = {};

		stream.stream1.rootSignature = rootSignature;
		stream.stream1.VS = { info->vertexShader->GetByteCode().data(), info->vertexShader->GetByteCode().size() };
		if (info->fragmentShader != nullptr)
		{
			stream.stream1.PS = { info->fragmentShader->GetByteCode().data(), info->fragmentShader->GetByteCode().size() };
		}

		// Blend State + RenderTargetFormats and SampleMask
		CD3DX12_BLEND_DESC blendState = {};
		blendState.AlphaToCoverageEnable = info->blendState.alphaToCoverageEnable;
		blendState.IndependentBlendEnable = info->blendState.independentBlendEnable;
		for (uint32_t i = 0; i < kMaxColorAttachments; ++i)
		{
            if (info->colorFormats[i] == PixelFormat::Undefined)
                break;

			RTVFormats.RTFormats[RTVFormats.NumRenderTargets++] = ToDXGIFormat(info->colorFormats[i]);

			const RenderTargetBlendState& renderTarget = info->blendState.renderTargets[i];
			blendState.RenderTarget[i].BlendEnable = EnableBlend(renderTarget);
			blendState.RenderTarget[i].SrcBlend = D3D12Blend(renderTarget.srcBlend);
			blendState.RenderTarget[i].DestBlend = D3D12Blend(renderTarget.destBlend);
			blendState.RenderTarget[i].BlendOp = D3D12BlendOperation(renderTarget.blendOperation);
			blendState.RenderTarget[i].SrcBlendAlpha = D3D12AlphaBlend(renderTarget.srcBlendAlpha);
			blendState.RenderTarget[i].DestBlendAlpha = D3D12AlphaBlend(renderTarget.destBlendAlpha);
			blendState.RenderTarget[i].BlendOpAlpha = D3D12BlendOperation(renderTarget.blendOperationAlpha);
			blendState.RenderTarget[i].RenderTargetWriteMask = D3D12RenderTargetWriteMask(renderTarget.writeMask);
			blendState.RenderTarget[i].LogicOpEnable = false;
			blendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;
		}
		stream.stream1.BlendState = blendState;
		stream.stream1.SampleMask = UINT_MAX; // pipeline->GetSampleMask();

		CD3DX12_RASTERIZER_DESC rasterizerState = {};
		rasterizerState.FillMode = ToD3D12(info->rasterizerState.fillMode);
		rasterizerState.CullMode = ToD3D12(info->rasterizerState.cullMode);
		rasterizerState.FrontCounterClockwise = (info->rasterizerState.frontFace == FaceWinding::CounterClockwise) ? TRUE : FALSE;
		rasterizerState.DepthBias = FloorToInt(info->rasterizerState.depthBias * (float)(1 << 24));;
		rasterizerState.DepthBiasClamp = info->rasterizerState.depthBiasClamp;
		rasterizerState.SlopeScaledDepthBias = info->rasterizerState.depthBiasSlopeScale;
		rasterizerState.DepthClipEnable = TRUE;
		rasterizerState.MultisampleEnable = info->sampleCount > SampleCount::Count1 ? TRUE : FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;
		rasterizerState.ForcedSampleCount = 0;
		rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		stream.stream1.RasterizerState = rasterizerState;

		// DepthStencilState
		CD3DX12_DEPTH_STENCIL_DESC d3d12DepthStencilState = {};
		d3d12DepthStencilState.DepthEnable = (info->depthStencilState.depthCompare == CompareFunction::Always && !info->depthStencilState.depthWriteEnabled);
		d3d12DepthStencilState.DepthWriteMask = info->depthStencilState.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		d3d12DepthStencilState.DepthFunc = ToD3D12ComparisonFunc(info->depthStencilState.depthCompare);
		d3d12DepthStencilState.StencilEnable = FALSE;
		d3d12DepthStencilState.StencilReadMask = info->depthStencilState.stencilReadMask;
		d3d12DepthStencilState.StencilWriteMask = info->depthStencilState.stencilWriteMask;

		d3d12DepthStencilState.FrontFace.StencilFailOp = ToD3D12(info->depthStencilState.frontFace.failOperation);
		d3d12DepthStencilState.FrontFace.StencilDepthFailOp = ToD3D12(info->depthStencilState.frontFace.depthFailOperation);
		d3d12DepthStencilState.FrontFace.StencilPassOp = ToD3D12(info->depthStencilState.frontFace.passOperation);
		d3d12DepthStencilState.FrontFace.StencilFunc = ToD3D12ComparisonFunc(info->depthStencilState.frontFace.compareFunction);

		d3d12DepthStencilState.BackFace.StencilFailOp = ToD3D12(info->depthStencilState.backFace.failOperation);
		d3d12DepthStencilState.BackFace.StencilDepthFailOp = ToD3D12(info->depthStencilState.backFace.depthFailOperation);
		d3d12DepthStencilState.BackFace.StencilPassOp = ToD3D12(info->depthStencilState.backFace.passOperation);
		d3d12DepthStencilState.BackFace.StencilFunc = ToD3D12ComparisonFunc(info->depthStencilState.backFace.compareFunction);
		stream.stream1.DepthStencilState = d3d12DepthStencilState;

		// InputLayout
		std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
		D3D12_INPUT_LAYOUT_DESC inputLayout = {};

		for (uint32_t location = 0; location < kMaxVertexAttributes; location++) {
			const VertexAttribute* attribute = &info->vertexLayout.attributes[location];
			if (attribute->format == VertexFormat::Invalid) {
				continue;
			}

			auto& element = elements.emplace_back();;
			element.SemanticName = "ATTRIBUTE";
			element.SemanticIndex = location;
			element.Format = ToD3D12(attribute->format);
			element.InputSlot = attribute->bufferIndex;
			element.AlignedByteOffset = attribute->offset;

			if (info->vertexLayout.buffers[attribute->bufferIndex].stepRate == VertexStepRate::Vertex)
			{
				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				element.InstanceDataStepRate = 0;
			}
			else
			{
				element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
				element.InstanceDataStepRate = 1;
			}

			vboSlotsUsed = Max(attribute->bufferIndex + 1, vboSlotsUsed);
			vboStrides[attribute->bufferIndex] = info->vertexLayout.buffers[attribute->bufferIndex].stride;
			inputLayout.NumElements++;
		}

		inputLayout.pInputElementDescs = elements.data();
		stream.stream1.InputLayout = inputLayout;
		stream.stream1.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

		// PrimitiveTopologyType
		primitiveTopology = ToD3D12PrimitiveTopology(info->primitiveTopology);
		switch (info->primitiveTopology)
		{
		case PrimitiveTopology::PointList:
			stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			break;
		case PrimitiveTopology::LineList:
		case PrimitiveTopology::LineStrip:
			stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			break;
		case PrimitiveTopology::TriangleList:
		case PrimitiveTopology::TriangleStrip:
			stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			break;
			//case PrimitiveTopology::PatchList:
			//	stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	//			break;
		default:
			stream.stream1.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
			break;
		}

		stream.stream1.RTVFormats = RTVFormats;
		stream.stream1.DSVFormat = ToDXGIFormat(info->depthStencilFormat);

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = D3D12SampleCount(info->sampleCount);
		sampleDesc.Quality = 0;
		stream.stream1.SampleDesc = sampleDesc;

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
		streamDesc.pPipelineStateSubobjectStream = &stream;
		streamDesc.SizeInBytes = sizeof(stream.stream1);
		//if (device.GetCaps().features.meshShader)
		//{
		//	streamDesc.SizeInBytes += sizeof(stream.stream2);
		//}

		HRESULT hr = device.GetHandle()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&handle));
		if (FAILED(hr))
		{
            LOGE("Failed to create RenderPipeline");
			return;
		}

		OnCreated();

		if (info->label != nullptr)
		{
			auto name = ToUtf16(info->label, strlen(info->label));
			handle->SetName(name.c_str());
		}
	}

	D3D12Pipeline::D3D12Pipeline(D3D12Graphics& device_, const ComputePipelineCreateInfo* info)
		: Pipeline(Type::ComputePipeline)
		, device(device_)
	{
		struct PSO_STREAM
		{
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_CS CS;
		} stream;

		stream.pRootSignature = nullptr;
		stream.CS = { info->shader->GetByteCode().data(), info->shader->GetByteCode().size() };

		D3D12_PIPELINE_STATE_STREAM_DESC streamDesc = {};
		streamDesc.pPipelineStateSubobjectStream = &stream;
		streamDesc.SizeInBytes = sizeof(stream);

		HRESULT hr = device.GetHandle()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&handle));
		if (FAILED(hr))
		{
            LOGE("Failed to create ComputePipeline");
			return;
		}

		OnCreated();

		if (info->label != nullptr)
		{
			auto name = ToUtf16(info->label, strlen(info->label));
			handle->SetName(name.c_str());
		}
	}

	D3D12Pipeline::~D3D12Pipeline()
	{
		Destroy();
	}

	void D3D12Pipeline::Destroy()
	{
		if (rootSignature != nullptr)
		{
			device.DeferDestroy(rootSignature);
			rootSignature = nullptr;
		}

        if (handle != nullptr)
        {
            device.DeferDestroy(handle);
            handle = nullptr;
        }

        OnDestroyed();
	}
}

