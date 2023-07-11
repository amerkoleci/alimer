// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;

namespace Alimer;

/// <summary>
/// Defines an application view.
/// </summary>
public abstract class AppView : ISwapChainSurface
{
    public event EventHandler? SizeChanged;

    protected AppView()
    {

    }

    public abstract bool IsMinimized { get; }
    public abstract SizeF ClientSize { get; }

    /// <inheritdoc />
    public abstract SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public abstract nint ContextHandle { get; }

    /// <inheritdoc />
    public abstract nint Handle { get; }

    /// <inheritdoc />
    SizeF ISwapChainSurface.Size => ClientSize;

    public SwapChain? SwapChain { get; private set; }
    public PixelFormat ColorFormat { get; set; } = PixelFormat.Bgra8UnormSrgb;
    public PixelFormat DepthStencilFormat { get; set; } = PixelFormat.Depth32Float;
    public Texture? DepthStencilTexture { get; private set; }

    public void CreateSwapChain(GraphicsDevice device)
    {
        SwapChainDescription description = new(ColorFormat);
        SwapChain = device.CreateSwapChain(this, description);
        ColorFormat = SwapChain.ColorFormat;

        if (DepthStencilFormat != PixelFormat.Undefined)
        {
            TextureDescription depthStencilTextureDesc = TextureDescription.Texture2D(DepthStencilFormat, (uint)ClientSize.Width, (uint)ClientSize.Height, usage: TextureUsage.RenderTarget);
            DepthStencilTexture = device.CreateTexture(in depthStencilTextureDesc);
        }
    }

    protected virtual void OnSizeChanged()
    {
        DepthStencilTexture?.Dispose();

        SizeChanged?.Invoke(this, EventArgs.Empty);
    }

    internal void Destroy()
    {
        DepthStencilTexture?.Dispose();
        SwapChain?.Dispose();
    }
}
