// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "Sample.h"

void Sample::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    if (depthStencilFormat != PixelFormat::Undefined)
    {
        TextureDescriptor depthStencilTextureDesc = TextureDescriptor::Texture2D(depthStencilFormat, windowSize.x, windowSize.y, 1u, 1u, TextureUsage::RenderTarget);
        _depthStencilTexture = device->CreateTexture(depthStencilTextureDesc);
    }
}
