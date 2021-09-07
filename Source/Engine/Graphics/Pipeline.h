// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/RefCount.h"
#include "Graphics/GraphicsDefs.h"

namespace Alimer
{

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

    ALIMER_API bool StencilTestEnabled(const DepthStencilState* depthStencil);
}

