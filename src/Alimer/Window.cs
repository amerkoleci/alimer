// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer;

/// <summary>
/// Defines an application window.
/// </summary>
public sealed partial class Window : ISwapChainSurface
{
    private string _title;
    public event EventHandler? SizeChanged;

    /// <summary>
    /// Gets a value indicating whether the window is currently minimized.
    /// </summary>
    public partial bool IsMinimized { get; }

    /// <summary>
    /// Gets or sets a value indicating whether the application is displayed in fullscreen mode.
    /// </summary>
    public partial bool IsFullscreen { get; set; }

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
                SetTitle(value);
            }
        }
    }

    #region ISwapChainSurface Members
    /// <inheritdoc />
    public SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public nint ContextHandle { get; }

    /// <inheritdoc />
    public nint Handle { get; }

    /// <inheritdoc />
    SizeI ISwapChainSurface.Size => ClientSize;
    #endregion

    public partial PointI Position { get; set; }
    public partial SizeI ClientSize { get; }

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

    private void OnSizeChanged()
    {
        DepthStencilTexture?.Dispose();
        SizeChanged?.Invoke(this, EventArgs.Empty);
    }

    private partial void SetTitle(string title);

    internal void Destroy()
    {
        DepthStencilTexture?.Dispose();
        SwapChain?.Dispose();
    }
}
