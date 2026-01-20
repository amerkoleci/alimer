// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class SwapChain : GraphicsObject
{
    public SwapChain(in SwapChainDescriptor descriptor)
        : base(descriptor.Label)
    {
        Surface = descriptor.Surface;
        Width = descriptor.Width;
        Height = descriptor.Height;
        ColorFormat = descriptor.Format;
        PresentMode = descriptor.PresentMode;
    }

    public SwapChainSurface Surface { get; }

    public uint Width { get; private set; }
    public uint Height { get; private set; }
    public PixelFormat ColorFormat { get; protected set; }
    public PresentMode PresentMode { get; }

    public void Resize(uint width, uint height)
    {
        if (Width == width && Height == height)
            return;

        ResizeBackBuffer();
        Width = width;
        Height = height;
    }

    protected abstract void ResizeBackBuffer();

    public abstract Texture? GetCurrentTexture();
}
