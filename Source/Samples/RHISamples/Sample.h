// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <Alimer/Alimer.h>
using namespace Alimer;

class Sample
{
public:
    virtual ~Sample() = default;

    virtual void Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat = PixelFormat::Depth32Float);
    virtual void Update(float deltaTime) {}
    virtual void Draw([[maybe_unused]] RHICommandBuffer* commandBuffer, [[maybe_unused]] RHITexture* outputTexture) = 0;

protected:
    RHITextureRef _depthStencilTexture;
};
