// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "DrawIndexedQuad.h"
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

void DrawIndexedQuad::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    Sample::Initialize(device, windowSize, colorFormat, depthStencilFormat);

    const VertexPositionColor vertices[] = {
        {Vector3(-0.5f, 0.5f, 0.5f), Colors::Blue},
        {Vector3(0.5f,  0.5f, 0.5f), Colors::Red},
        {Vector3(0.5f, -0.5f, 0.5f), Colors::Lime},
        {Vector3(-0.5f, -0.5f, 0.5f), Colors::Yellow},
    };

    _vertexBuffer = RHICreateBuffer(device, vertices, 4, RHIBufferUsage::Vertex);

    const uint16_t indices[] = {
        0, 2, 1,    // first triangle
        0, 3, 2,    // second triangle
    };
    _indexBuffer = RHICreateBuffer(device, indices, 6, RHIBufferUsage::Index);

    std::vector<ShaderMacro> macros = {
        { "VARIANT", "0" }
    };
    RHIShaderModuleRef vertexShader = RHILoadShader(device, RHIShaderStages::Vertex, "Triangle");
    RHIShaderModuleRef fragmentShader = RHILoadShader(device, RHIShaderStages::Fragment, "Triangle", &macros);

    std::array<VertexAttribute, 2> vertexAttributes = {
        VertexAttribute{ VertexAttributeSemantic::Position, VertexAttributeFormat::Float32x3, offsetof(VertexPositionColor, position) },
        VertexAttribute{ VertexAttributeSemantic::Color, VertexAttributeFormat::Unorm8x4, offsetof(VertexPositionColor, color) }
    };

    VertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.attributeCount = static_cast<uint32_t>(vertexAttributes.size());
    vertexBufferLayout.attributes = vertexAttributes.data();

    RHIRenderPipelineDesc pipelineDesc{};
    pipelineDesc.label = "IndexedQuad";
    pipelineDesc.vertexBufferLayoutCount = 1u;
    pipelineDesc.vertexBufferLayouts = &vertexBufferLayout;
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.fragmentShader = fragmentShader;
    pipelineDesc.colorAttachmentFormats[0] = colorFormat;
    pipelineDesc.depthStencilFormat = depthStencilFormat;
    _renderPipeline = device->CreateRenderPipeline(pipelineDesc);
}

void DrawIndexedQuad::Draw(CommandBuffer* commandBuffer, RHITexture* outputTexture)
{
    RenderPassColorAttachment colorAttachment;
    colorAttachment.view = outputTexture->GetDefaultView();
    colorAttachment.loadAction = LoadAction::Clear;
    colorAttachment.storeAction = StoreAction::Store;
    //colorAttachment.initialState = ResourceState::RenderTarget;
    //colorAttachment.finalState = ResourceState::CopySource;
    colorAttachment.clearColor = { { 0.3f, 0.3f, 0.3f, 1.0f } };

    RenderPassDepthStencilAttachment depthStencilAttachment;
    if (_depthStencilTexture)
    {
        depthStencilAttachment.view = _depthStencilTexture->GetDefaultView();
        depthStencilAttachment.depthClearValue = 1.0f;
        //depthStencilAttachment.depthClearValue = 0.0f; // Infinite reverse Z
    }

    RenderPassDesc renderPassDescriptor = {};
    renderPassDescriptor.colorAttachmentCount = 1u;
    renderPassDescriptor.colorAttachments = &colorAttachment;
    if (_depthStencilTexture)
    {
        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
    }

    RenderPassEncoder* renderPass = commandBuffer->BeginRenderPass(renderPassDescriptor);
    renderPass->SetPipeline(_renderPipeline.Get());
    renderPass->SetVertexBuffer(0, _vertexBuffer.Get());
    renderPass->SetIndexBuffer(_indexBuffer.Get(), 0, IndexFormat::Uint16);
    struct PushData
    {
        Color color = Colors::Orange;
    } data;

    renderPass->SetPushConstants(data);
    renderPass->DrawIndexed(6);
    renderPass->End();
}
