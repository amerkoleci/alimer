// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Graphics/GraphicsDefs.h"

namespace Alimer
{


    enum class VertexFormat : uint32_t
    {
        Undefined = 0,
        UChar2,
        UChar4,
        Char2,
        Char4,
        UChar2Norm,
        UChar4Norm,
        Char2Norm,
        Char4Norm,
        UShort2,
        UShort4,
        Short2,
        Short4,
        UShort2Norm,
        UShort4Norm,
        Short2Norm,
        Short4Norm,
        Half2,
        Half4,
        Float,
        Float2,
        Float3,
        Float4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
        Int,
        Int2,
        Int3,
        Int4,
        RGB10A2Unorm
    };

    enum class VertexStepRate : uint32_t
    {
        Vertex = 0,
        Instance
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

    struct VertexLayout
    {
        VertexBufferLayout buffers[kMaxVertexBufferBindings];
        VertexAttribute attributes[kMaxVertexAttributes];
    };

    struct RenderTargetBlendState
    {
        bool blendEnable = false;
        BlendFactor srcBlend = BlendFactor::One;
        BlendFactor destBlend = BlendFactor::Zero;
        BlendOperation blendOp = BlendOperation::Add;
        BlendFactor srcBlendAlpha = BlendFactor::One;
        BlendFactor destBlendAlpha = BlendFactor::Zero;
        BlendOperation blendOpAlpha = BlendOperation::Add;
        ColorWriteMask writeMask = ColorWriteMask::All;
    };

    struct BlendState
    {
        bool alphaToCoverageEnable = false;
        bool independentBlendEnable = false;

        RenderTargetBlendState renderTargets[kMaxColorAttachments] = {};
    };

    struct StencilFaceState
    {
        StencilOperation failOp = StencilOperation::Keep;
        StencilOperation passOp = StencilOperation::Keep;
        StencilOperation depthFailOp = StencilOperation::Keep;
        CompareFunction compare = CompareFunction::Always;
    };

    struct DepthStencilState
    {
        bool depthWriteEnable = true;
        CompareFunction depthCompare = CompareFunction::Less;
        uint8_t stencilReadMask = 0xFF;
        uint8_t stencilWriteMask = 0xFF;
        StencilFaceState frontFace;
        StencilFaceState backFace;
    };

    struct RasterizerState
    {
        CullMode cullMode = CullMode::Back;
        FaceWinding frontFace = FaceWinding::Clockwise;
        FillMode fillMode = FillMode::Solid;
        float depthBias = 0.0f;
        float depthBiasSlopeScale = 0.0f;
        float depthBiasClamp = 0.0f;
    };

    struct RenderPipelineDesc
    {
        std::string label;
        Shader* vertex = nullptr;
        Shader* hull = nullptr;
        Shader* domain = nullptr;
        Shader* geometry = nullptr;
        Shader* pixel = nullptr;
        //IShader* mesh = nullptr;
        //IShader* amplification = nullptr;

        VertexLayout            vertexLayout;
        BlendState              blendState;
        DepthStencilState       depthStencilState;
        RasterizerState         rasterizerState;
        PrimitiveTopology       primitiveTopology = PrimitiveTopology::TriangleList;
        uint32_t                patchControlPoints = 0;
        PixelFormat             colorFormats[kMaxColorAttachments] = {};
        PixelFormat             depthStencilFormat = PixelFormat::Undefined;
        uint32_t                sampleCount = 1;
    };

    class ALIMER_API Pipeline : public RefCounted
    {
    public:
        enum class Type
        {
            RenderPipeline,
            ComputePipeline,
            RaytracingPipeline,
        };

        /// Create new render pipeline.
        [[nodiscard]] static PipelineRef Create(const RenderPipelineDesc& desc);

    protected:
        /// Constructor.
        Pipeline(Type type);

        Type type;
    };

    ALIMER_API uint32_t GetVertexFormatNumComponents(VertexFormat format);
    ALIMER_API uint32_t GetVertexFormatComponentSize(VertexFormat format);
    ALIMER_API uint32_t GetVertexFormatSize(VertexFormat format);
    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);
}

