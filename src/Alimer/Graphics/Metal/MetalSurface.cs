// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Metal;

internal sealed class MetalSurface : Surface
{
    public MetalSurface(in SurfaceDescriptor descriptor)
        : base(descriptor)
    {
        if (Source is not MetalLayerChainSurface metalSurface)
        {
            throw new GraphicsException($"Metal: Invalid kind for surface: {Source.Kind}");
        }

        Layer = metalSurface.Layer;
    }

    public nint Layer { get; }

    protected override void ConfigureCore()
    {
    }

    protected override void UnconfigureCore()
    {
    }

    protected override void ResizeCore(int newWidth, int newHeight)
    {
    }

    public override Texture? AcquireNextTexture() => null;

    protected internal override void Destroy()
    {
    }
}
