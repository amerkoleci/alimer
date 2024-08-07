// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Mathematics;

namespace Alimer.Graphics;

public abstract class SwapChain : GraphicsObject
{
    public SwapChain(ISwapChainSurface surface, in SwapChainDescription description)
        : base(description.Label)
    {
        Surface = surface;
        Surface.SizeChanged += OnSurfaceSizeChanged;
        ColorFormat = description.Format;
        PresentMode = description.PresentMode;
        IsFullscreen = description.IsFullscreen;
    }

    public ISwapChainSurface Surface { get; }

    public PixelFormat ColorFormat { get; protected set; }
    public PresentMode PresentMode { get; }
    public bool IsFullscreen { get; protected set; }

    public bool AutoResizeDrawable { get; set; } = true;
    public SizeI DrawableSize => Surface.Size;

    protected abstract void ResizeBackBuffer();

    private void OnSurfaceSizeChanged(object? sender, EventArgs eventArgs)
    {
        ResizeBackBuffer();
    }

    public abstract Texture? GetCurrentTexture();
}
