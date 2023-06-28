// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;

namespace Alimer.Graphics;

public abstract class SwapChain : GraphicsObject
{
    public SwapChain(GraphicsDevice device, SwapChainSurface surface, in SwapChainDescriptor descriptor)
        : base(device, descriptor.Label)
    {
        Surface = surface;
        ColorFormat = descriptor.Format;
        PresentMode = descriptor.PresentMode;
        IsFullscreen = descriptor.IsFullscreen;
        DrawableSize = new(descriptor.Width, descriptor.Height);
    }

    public SwapChainSurface Surface { get; }

    public PixelFormat ColorFormat { get; protected set; }
    public PresentMode PresentMode { get; }
    public bool IsFullscreen { get; protected set; }    

    public bool AutoResizeDrawable { get; set; } = true;
    public Size DrawableSize { get; protected set; }

    protected abstract void ResizeBackBuffer(int width, int height);
}
