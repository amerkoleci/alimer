// Copyright (c) Amer Koleci and Contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

#include "Sample.h"

void Sample::Initialize(RHIDevice* device, const UInt2& windowSize, PixelFormat colorFormat, PixelFormat depthStencilFormat)
{
    _device = device;
    _colorFormat = colorFormat;
    _depthStencilFormat = depthStencilFormat;
    Resize(windowSize);
}

void Sample::Resize(const UInt2& windowSize)
{
    if (_depthStencilFormat != PixelFormat::Undefined)
    {
        TextureDescriptor depthStencilTextureDesc = TextureDescriptor::Texture2D(_depthStencilFormat, windowSize.x, windowSize.y, 1u, 1u, TextureUsage::RenderTarget);
        _depthStencilTexture = _device->CreateTexture(depthStencilTextureDesc);
    }
}
