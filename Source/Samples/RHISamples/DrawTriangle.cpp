// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "DrawTriangle.h"
#include <array>

struct VertexPositionColor
{
    VertexPositionColor() = default;

    VertexPositionColor(const VertexPositionColor&) = default;
    VertexPositionColor& operator=(const VertexPositionColor&) = default;

    VertexPositionColor(VertexPositionColor&&) = default;
    VertexPositionColor& operator=(VertexPositionColor&&) = default;

    VertexPositionColor(const Vector3& position_, const Color& color_) noexcept
        : position(position_)
        , color(color_.ToRgba())
    {
    }

    Vector3 position;
    ColorRgba color;
};

void DrawTriangle::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    Sample::Initialize(device, windowSize, colorFormat, depthStencilFormat);

    const VertexPositionColor vertices[] = {
            {Vector3(0.0f, 0.5f, 0.f), Colors::Red},
            {Vector3(-0.5f, -0.5f, 0.f), Colors::Lime},
            {Vector3(0.5f, -0.5f, 0.f), Colors::Blue}
    };

    _vertexBuffer = RHICreateBuffer(device, vertices, 3u, RHIBufferUsage::Vertex);

    std::vector<RHIShaderMacro> macros = {
        { "VARIANT", "0" }
    };
    RHIShaderModuleRef vertexShader = RHILoadShader(device, RHIShaderStages::Vertex, "Triangle");
    RHIShaderModuleRef fragmentShader = RHILoadShader(device, RHIShaderStages::Fragment, "Triangle", macros);

    std::array<RHIVertexAttribute, 2> vertexAttributes = {
        RHIVertexAttribute{ RHIVertexAttributeSemantic::Position, RHIVertexAttributeFormat::Float32x3, offsetof(VertexPositionColor, position) },
        RHIVertexAttribute{ RHIVertexAttributeSemantic::Color, RHIVertexAttributeFormat::Unorm8x4, offsetof(VertexPositionColor, color) }
    };

    RHIVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttributes.size());
    vertexBufferLayout.attributes = vertexAttributes.data();

    RHIRenderPipelineDesc pipelineDesc{};
    pipelineDesc.label = "Triangle";
    pipelineDesc.vertexBufferLayoutCount = 1u;
    pipelineDesc.vertexBufferLayouts = &vertexBufferLayout;
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.fragmentShader = fragmentShader;
    pipelineDesc.colorAttachmentFormats[0] = colorFormat;
    pipelineDesc.depthStencilFormat = depthStencilFormat;
    _renderPipeline = device->CreateRenderPipeline(pipelineDesc);
}

void DrawTriangle::Draw(RHICommandBuffer* commandBuffer, RHITexture* outputTexture)
{
    RHIRenderPassColorAttachment colorAttachment;
    colorAttachment.view = outputTexture->GetDefaultView();
    colorAttachment.loadAction = RHILoadAction::Clear;
    colorAttachment.storeAction = RHIStoreAction::Store;
    //colorAttachment.initialState = ResourceState::RenderTarget;
    //colorAttachment.finalState = ResourceState::CopySource;
    colorAttachment.clearColor = { { 0.3f, 0.3f, 0.3f, 1.0f } };

    RHIRenderPassDepthStencilAttachment depthStencilAttachment;
    if (_depthStencilTexture)
    {
        depthStencilAttachment.view = _depthStencilTexture->GetDefaultView();
        depthStencilAttachment.depthClearValue = 1.0f;
        //depthStencilAttachment.depthClearValue = 0.0f; // Infinite reverse Z
    }

    RHIRenderPassDesc renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1u;
    renderPassDescriptor.colorAttachments = &colorAttachment;
    if (_depthStencilTexture)
    {
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    }

    RHIRenderPassEncoder* renderPass = commandBuffer->BeginRenderPass(renderPassDescriptor);
    renderPass->SetPipeline(_renderPipeline.Get());
    renderPass->SetVertexBuffer(0, _vertexBuffer.Get());
    struct PushData
    {
        Color color = Colors::Orange;
    } data;

    renderPass->SetPushConstants(data);
    renderPass->Draw(3);
    renderPass->End();
}
