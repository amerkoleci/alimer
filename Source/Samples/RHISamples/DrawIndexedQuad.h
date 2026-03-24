// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Sample.h"

class DrawIndexedQuad final : public Sample
{
public:
    void Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat) override;
    void Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture) override;

private:
    RHIBufferRef _vertexBuffer;
    RHIBufferRef _indexBuffer;
    RenderPipelineRef _renderPipeline;
};

