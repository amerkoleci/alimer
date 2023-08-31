// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using TerraFX.Interop.DirectX;

namespace Alimer.Graphics;

public abstract class SwapChain : GraphicsObject
{
    public SwapChain(ISwapChainSurface surface, in SwapChainDescription descriptor)
        : base(descriptor.Label)
    {
        Surface = surface;
        Surface.SizeChanged += OnSurfaceSizeChanged;
        ColorFormat = descriptor.Format;
        PresentMode = descriptor.PresentMode;
        IsFullscreen = descriptor.IsFullscreen;
    }

    public ISwapChainSurface Surface { get; }

    public PixelFormat ColorFormat { get; protected set; }
    public PresentMode PresentMode { get; }
    public bool IsFullscreen { get; protected set; }

    public bool AutoResizeDrawable { get; set; } = true;
    public SizeF DrawableSize => Surface.Size;

    protected abstract void ResizeBackBuffer();

    private void OnSurfaceSizeChanged(object? sender, EventArgs eventArgs)
    {
        ResizeBackBuffer();
    }

    public abstract Texture? GetCurrentTexture();
}
