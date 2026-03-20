// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "DrawTriangle.h"

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
            {Vector3(0.0f, 0.5f, 0.5f), Colors::Red},
            {Vector3(0.5f, -0.5f, 0.5f), Colors::Lime},
            {Vector3(-0.5f, -0.5f, 0.5f), Colors::Blue}
    };

    _vertexBuffer = device->CreateBuffer(sizeof(vertices), BufferUsage::Vertex, vertices);


    std::vector<ShaderMacro> macros = {
        { "VARIANT", "0" }
    };
    RHIShaderModuleRef vertexShader = RHILoadShader(device, ShaderStages::Vertex, "Triangle");
    RHIShaderModuleRef fragmentShader = RHILoadShader(device, ShaderStages::Fragment, "Triangle", &macros);

    PipelineLayoutDesc pipelineLayoutDesc = {};
    _pipelineLayout = device->CreatePipelineLayout(pipelineLayoutDesc);

    RenderPipelineDesc pipelineDesc{};
    pipelineDesc.label = "Triangle";
    pipelineDesc.layout = _pipelineLayout;
    //pipelineDesc.vertexInput.bufferCount = 1u;
    //pipelineDesc.vertex.buffers = &vertexBufferLayout;
    pipelineDesc.vertexShader = vertexShader;
    pipelineDesc.fragmentShader = fragmentShader;
    pipelineDesc.colorAttachmentCount = 1u;
    pipelineDesc.colorAttachmentFormats = &colorFormat;
    pipelineDesc.depthStencilFormat = depthStencilFormat;
    _renderPipeline = device->CreateRenderPipeline(pipelineDesc);
}

void DrawTriangle::Draw(RHICommandBuffer* commandBuffer, RHITexture* outputTexture)
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

    RenderCommandEncoder* renderPass = commandBuffer->BeginRenderPass(renderPassDescriptor);
    renderPass->SetPipeline(_renderPipeline.Get());
    renderPass->SetVertexBuffer(0, _vertexBuffer.Get());
    renderPass->Draw(3);
    renderPass->End();
}
