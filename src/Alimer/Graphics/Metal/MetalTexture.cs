// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics.Metal;

internal unsafe sealed class MetalTexture : Texture
{
    private readonly MetalGraphicsDevice _device;

    public MetalTexture(MetalGraphicsDevice device, in TextureDescriptor descriptor, TextureData* initialData)
        : base(in descriptor)
    {
        _device = device;
        SetTextureLayout(TextureLayout.Undefined);
    }

    public override GraphicsDevice Device => _device;

    protected override TextureView CreateView(in TextureViewDescriptor descriptor)
        => new MetalTextureView(this, in descriptor);

    protected internal override void Destroy()
    {
        DestroyViews();
    }
}
