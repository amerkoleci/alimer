// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "DrawTriangle.h"

void DrawTriangle::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    Sample::Initialize(device, windowSize, colorFormat, depthStencilFormat);
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
    renderPass->End();
}
