// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer;

/// <summary>
/// Defines an application window.
/// </summary>
public abstract class Window : ISwapChainSurface
{
    public event EventHandler? SizeChanged;

    protected string _title = string.Empty;

    protected Window()
    {

    }

    /// <summary>
    /// Gets and sets the title of the window.
    /// </summary>
    public string Title
    {
        get => _title;
        set
        {
            Guard.IsNotNullOrEmpty(value, nameof(value));

            if (_title != value)
            {
                _title = value;
                SetTitle(_title);
            }
        }
    }

    #region ISwapChainSurface Members
    /// <inheritdoc />
    public abstract SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public abstract nint ContextHandle { get; }

    /// <inheritdoc />
    public abstract nint Handle { get; }

    /// <inheritdoc />
    SizeI ISwapChainSurface.Size => ClientSize;
    #endregion ISwapChainSurface Members

    public abstract bool IsMinimized { get; }

    /// <summary>
    /// Gets or Sets whether the Window is in Fullscreen Mode
    /// </summary>
    public abstract bool IsFullscreen { get; set; }
    public abstract System.Drawing.PointF Position { get; set; }
    public abstract SizeI ClientSize { get; }

    public SwapChain? SwapChain { get; private set; }
    public PixelFormat ColorFormat { get; set; } = PixelFormat.BGRA8UnormSrgb;
    public PixelFormat DepthStencilFormat { get; set; } = PixelFormat.Depth32Float;
    public Texture? DepthStencilTexture { get; private set; }

    /// <summary>
    /// Gets the view aspect ratio.
    /// </summary>
    public float AspectRatio
    {
        get
        {
            Size size = ClientSize;
            if (size.Width != 0 && size.Height != 0)
            {
                return size.Width / size.Height;
            }

            return 0.0f;
        }
    }

    public void CreateSwapChain(GraphicsDevice device)
    {
        SwapChainDescription description = new(ColorFormat);
        SwapChain = device.CreateSwapChain(this, description);
        ColorFormat = SwapChain.ColorFormat;

        if (DepthStencilFormat != PixelFormat.Undefined)
        {
            TextureDescription depthStencilTextureDesc = TextureDescription.Texture2D(DepthStencilFormat,
                (uint)ClientSize.Width, (uint)ClientSize.Height,
                usage: TextureUsage.RenderTarget
                );
            DepthStencilTexture = device.CreateTexture(in depthStencilTextureDesc);
        }
    }

    protected virtual void OnSizeChanged()
    {
        DepthStencilTexture?.Dispose();
        SizeChanged?.Invoke(this, EventArgs.Empty);
    }

    protected abstract void SetTitle(string title);

    protected internal void Destroy()
    {
        DepthStencilTexture?.Dispose();
        SwapChain?.Dispose();
    }
}
