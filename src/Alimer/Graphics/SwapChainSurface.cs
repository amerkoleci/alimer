// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// A platform-specific object representing a renderable surface.
/// A SwapChainSurface can be created with one of several static factory methods.
/// A SwapChainSurface is used to describe a Swapchain (see <see cref="SwapChainDescriptor"/>).
/// </summary>
public abstract class SwapChainSurface
{
    protected SwapChainSurface()
    {
    }

    /// <summary>
    /// Gets the surface kind.
    /// </summary>
    public abstract SwapChainSurfaceType Kind { get; }

    /// <summary>
    /// Creates a new instance of <see cref="SwapChainSurface"/> from the given Android information.
    /// </summary>
    /// <param name="display">The Wayland display proxy.</param>
    /// <param name="surface">The Wayland surface proxy to map.</param>
    /// <returns>A new SwapchainSource which can be used to create a <see cref="SwapChain"/> for the given Android window.
    /// </returns>
    public static SwapChainSurface CreateAndroid(nint window) => new AndroidSwapChainSurface(window);

    /// <summary>
    /// Creates a new instance of <see cref="SwapChainSurface"/> from the given Wayland information.
    /// </summary>
    /// <param name="display">The Wayland display proxy.</param>
    /// <param name="surface">The Wayland surface proxy to map.</param>
    /// <returns>A new SwapchainSource which can be used to create a <see cref="SwapChain"/> for the given Wayland surface.
    /// </returns>
    public static SwapChainSurface CreateWayland(nint display, nint surface) => new WaylandSwapChainSurface(display, surface);

    /// <summary>
    /// Creates a new instance of <see cref="SwapChainSurface"/> from the given Xlib information.
    /// </summary>
    /// <param name="display">An Xlib Display.</param>
    /// <param name="window">An Xlib Window.</param>
    /// <returns>A new of <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given Xlib window.
    /// </returns>
    public static SwapChainSurface CreateXlib(nint display, ulong window) => new XlibSwapChainSurface(display, window);

    /// <summary>
    /// Creates a new instance of <see cref="SwapChainSurface"/> for a CAMetalLayer instance.
    /// </summary>
    /// <param name="layer">The CAMetalLayer handle.</param>
    /// <returns>A new instance of <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given CAMetalLayer instance.
    /// </returns>
    public static SwapChainSurface CreateMetalLayer(nint layer) => new MetalLayerChainSurface(layer);

    /// <summary>
    /// Creates a new instance of <see cref="SwapChainSurface"/> for a Win32 window.
    /// </summary>
    /// <param name="hwnd">The Win32 window handle.</param>
    /// <returns>A new instance of <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given Win32 window.
    /// </returns>
    public static SwapChainSurface CreateWin32(nint hwnd) => new Win32SwapChainSurface(hwnd);
}

internal class AndroidSwapChainSurface(nint window) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Android;

    public nint Window { get; } = window;
}

internal class WaylandSwapChainSurface(nint display, nint surface) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Wayland;

    public nint Display { get; } = display;
    public nint Surface { get; } = surface;
}

internal class XlibSwapChainSurface(nint display, ulong window) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Xlib;

    public nint Display { get; } = display;
    public ulong Window { get; } = window;
}

internal class MetalLayerChainSurface(nint layer) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.MetalLayer;

    /// <summary>
    /// Instance of CAMetalLayer.
    /// </summary>
    public nint Layer { get; } = layer;
}

internal class Win32SwapChainSurface(nint hwnd) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Win32;

    public nint Hwnd { get; } = hwnd;
}

internal class SwapChainPanelChainSurface(nint swapChainPanel) : SwapChainSurface
{
    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.SwapChainPanel;

    public nint SwapChainPanel { get; } = swapChainPanel;
}
