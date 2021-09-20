// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GPUResource.h"

namespace Alimer
{
	enum class VertexStepRate : uint32_t
	{
		Vertex = 0,
		Instance = 1
	};

	enum class BlendFactor : uint32_t
    {
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationColor,
		OneMinusDestinationColor,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		SourceAlphaSaturated,
		BlendColor,
		OneMinusBlendColor,
		Source1Color,
		OneMinusSource1Color,
		Source1Alpha,
		OneMinusSource1Alpha,
	};

	enum class BlendOperation : uint32_t
    {
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class ColorWriteMask : uint8_t
    {
		None = 0,
		Red = 0x01,
		Green = 0x02,
		Blue = 0x04,
		Alpha = 0x08,
		All = 0x0F
	};
    ALIMER_DEFINE_ENUM_BITWISE_OPERATORS(ColorWriteMask);

	enum class StencilOperation : uint32_t
    {
		Keep,
		Zero,
		Replace,
		IncrementClamp,
		DecrementClamp,
		Invert,
		IncrementWrap,
		DecrementWrap,
	};

	enum class FillMode : uint32_t
    {
		Solid,
		Wireframe,
	};

	enum class CullMode : uint32_t
    {
		None,
		Front,
		Back
	};

	enum class FaceWinding : uint32_t
    {
		Clockwise,
		CounterClockwise,
	};

    enum class PrimitiveTopology : uint32_t
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        PatchList,
        Count
    };

	struct VertexBufferLayout
	{
        uint32_t stride = 0;
		VertexStepRate stepRate = VertexStepRate::Vertex;
	};

	struct VertexAttribute
	{
		VertexFormat format = VertexFormat::Undefined;
		uint32_t offset = 0;
        uint32_t bufferIndex = 0;
	};

    struct VertexLayout {
        VertexBufferLayout buffers[kMaxVertexBufferBindings];
        VertexAttribute attributes[kMaxVertexAttributes];
    };

	struct StencilFaceState {
		StencilOperation failOperation = StencilOperation::Keep;
		StencilOperation depthFailOperation = StencilOperation::Keep;
		StencilOperation passOperation = StencilOperation::Keep;
		CompareFunction compareFunction = CompareFunction::Always;
	};

	struct DepthStencilState {
		bool depthWriteEnabled = true;
		CompareFunction depthCompare = CompareFunction::LessEqual;
		StencilFaceState frontFace;
		StencilFaceState backFace;
		uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
	};

	struct RenderTargetBlendState {
		BlendFactor srcBlend = BlendFactor::One;
		BlendFactor destBlend = BlendFactor::Zero;
		BlendOperation blendOperation = BlendOperation::Add;
		BlendFactor srcBlendAlpha = BlendFactor::One;
		BlendFactor destBlendAlpha = BlendFactor::Zero;
		BlendOperation blendOperationAlpha = BlendOperation::Add;
		ColorWriteMask writeMask = ColorWriteMask::All;
	};

    struct BlendState {
        bool alphaToCoverageEnable{ false };
        bool independentBlendEnable{ false };

        RenderTargetBlendState renderTargets[kMaxColorAttachments] = {};
    };

    struct RasterizerState {
        CullMode cullMode = CullMode::Back;
        FaceWinding frontFace = FaceWinding::Clockwise;
        FillMode fillMode = FillMode::Solid;
        float depthBias = 0.0f;
        float depthBiasSlopeScale = 0.0f;
        float depthBiasClamp = 0.0f;
    };

	struct RenderPipelineStateCreateInfo
    {
		const char* label = nullptr;
		Shader* vertexShader;
		Shader* pixelShader = nullptr;

        VertexLayout        vertexLayout;
        BlendState          blendState;
		DepthStencilState   depthStencilState;
        RasterizerState     rasterizerState;
		PrimitiveTopology   primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t            patchControlPoints = 0;
        PixelFormat         colorFormats[kMaxColorAttachments] = {};
        PixelFormat         depthStencilFormat{ PixelFormat::Undefined };
		SampleCount         sampleCount = SampleCount::Count1;
	};

    struct ComputePipelineCreateInfo
    {
        const char* label = nullptr;
        Shader* shader;
    };

	ALIMER_API bool EnableBlend(const RenderTargetBlendState& state);
    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);

    enum class PipelineType
    {
        Render,
        Compute,
        Raytracing,
    };

	class ALIMER_API Pipeline : public RefCounted
	{
	public:
		/**
		* Create new render pipeline.
		* @param info - The render pipeline info.
		*/
		[[nodiscard]] static PipelineRef Create(const RenderPipelineStateCreateInfo& info);

		/**
		* Create new compute pipeline.
		* @param info - The compute pipeline info.
		*/
		[[nodiscard]] static PipelineRef Create(const ComputePipelineCreateInfo& info);

        [[nodiscard]] virtual PipelineType GetType() const = 0;
	};
}
