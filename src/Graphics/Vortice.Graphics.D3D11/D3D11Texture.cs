// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Microsoft.Toolkit.Diagnostics;
using Vortice.Direct3D11;

namespace Vortice.Graphics;

internal unsafe class D3D11Texture : Texture
{
    public D3D11Texture(D3D11GraphicsDevice device, in TextureDescriptor descriptor)
        : base(device, descriptor)
    {
        Texture2DDescription texture2DDesc = new()
        {
            Width = descriptor.Width,
            Height = descriptor.Height,
            ArraySize = descriptor.DepthOrArraySize,
            MipLevels = descriptor.MipLevels,
            Format = descriptor.Format.ToDXGIFormat(),
            SampleDescription = new(descriptor.SampleCount.ToD3D(), 0),
        };

        if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
        {
            texture2DDesc.BindFlags |= BindFlags.UnorderedAccess;
        }

        // If depth and either sampled or storage, set to typeless
        if (descriptor.Format.IsDepthFormat())
        {
            if ((descriptor.Usage & TextureUsage.ShaderRead) != 0
                || (descriptor.Usage & TextureUsage.ShaderWrite) != 0)
            {
                texture2DDesc.Format = descriptor.Format.GetTypelessFormatFromDepthFormat();
            }
        }

        switch (descriptor.Dimension)
        {
            case TextureDimension.Texture2D:
                Handle = device.NativeDevice.CreateTexture2D(texture2DDesc, null);
                break;

            default:
                ThrowHelper.ThrowArgumentException<TextureDimension>("Invalid texture dimension");
                return;
        }
    }

    public ID3D11Resource Handle { get; }

    //public D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Handle.Dispose();
        }
    }
}
