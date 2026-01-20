// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer;

/// <summary>
/// Defines an application window.
/// </summary>
public sealed partial class Window 
{
    private string _title;
    public event EventHandler? SizeChanged;

    /// <summary>
    /// Gets the swap chain surface associated with this instance.
    /// </summary>
    public partial SwapChainSurface Surface { get; }

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

    public partial PointI Position { get; set; }
    public partial SizeI ClientSize { get; }

    public SwapChain? SwapChain { get; private set; }
    public PixelFormat ColorFormat { get; set; } = PixelFormat.BGRA8UnormSrgb;

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
        SwapChainDescriptor description = new(Surface, (uint)ClientSize.Width, (uint)ClientSize.Height, ColorFormat);
        SwapChain = device.CreateSwapChain(description);
        ColorFormat = SwapChain.ColorFormat;

    }

    private void OnSizeChanged()
    {
        SizeChanged?.Invoke(this, EventArgs.Empty);
    }

    private partial void SetTitle(string title);

    internal void Destroy()
    {
        SwapChain?.Dispose();
    }
}
