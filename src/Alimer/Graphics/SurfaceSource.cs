// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// A platform-specific object representing a renderable surface.
/// A <see cref="SurfaceSource"/> can be created with one of several static factory methods.
/// A <see cref="SurfaceSource"/> is used to describe a <see cref="Surface"/>.
/// </summary>
public abstract class SurfaceSource
{
    protected SurfaceSource()
    {
    }

    /// <summary>
    /// Gets the surface kind.
    /// </summary>
    public abstract SurfaceSourceType Kind { get; }

    /// <summary>
    /// Creates a new instance of <see cref="SurfaceSource"/> from the given Android information.
    /// </summary>
    /// <param name="display">The Wayland display proxy.</param>
    /// <param name="surface">The Wayland surface proxy to map.</param>
    /// <returns>A new SwapchainSource which can be used to create a <see cref="Surface"/> for the given Android window.
    /// </returns>
    public static SurfaceSource CreateAndroid(nint window) => new AndroidSwapChainSurface(window);

    /// <summary>
    /// Creates a new instance of <see cref="SurfaceSource"/> from the given Wayland information.
    /// </summary>
    /// <param name="display">The Wayland display proxy.</param>
    /// <param name="surface">The Wayland surface proxy to map.</param>
    /// <returns>A new SwapchainSource which can be used to create a <see cref="Surface"/> for the given Wayland surface.
    /// </returns>
    public static SurfaceSource CreateWayland(nint display, nint surface) => new WaylandSwapChainSurface(display, surface);

    /// <summary>
    /// Creates a new instance of <see cref="SurfaceSource"/> from the given Xlib information.
    /// </summary>
    /// <param name="display">An Xlib Display.</param>
    /// <param name="window">An Xlib Window.</param>
    /// <returns>A new of <see cref="SurfaceSource"/> which can be used to create a <see cref="Surface"/> for the given Xlib window.
    /// </returns>
    public static SurfaceSource CreateXlib(nint display, ulong window) => new XlibSwapChainSurface(display, window);

    /// <summary>
    /// Creates a new instance of <see cref="SurfaceSource"/> for a CAMetalLayer instance.
    /// </summary>
    /// <param name="layer">The CAMetalLayer handle.</param>
    /// <returns>A new instance of <see cref="SurfaceSource"/> which can be used to create a <see cref="Surface"/> for the given CAMetalLayer instance.
    /// </returns>
    public static SurfaceSource CreateMetalLayer(nint layer) => new MetalLayerChainSurface(layer);

    /// <summary>
    /// Creates a new instance of <see cref="SurfaceSource"/> for a Win32 window.
    /// </summary>
    /// <param name="hwnd">The Win32 window handle.</param>
    /// <returns>A new instance of <see cref="SurfaceSource"/> which can be used to create a <see cref="Surface"/> for the given Win32 window.
    /// </returns>
    public static SurfaceSource CreateWin32(nint hwnd) => new Win32SwapChainSurface(hwnd);
}

internal class AndroidSwapChainSurface(nint window) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.Android;

    public nint Window { get; } = window;
}

internal class WaylandSwapChainSurface(nint display, nint surface) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.Wayland;

    public nint Display { get; } = display;
    public nint Surface { get; } = surface;
}

internal class XlibSwapChainSurface(nint display, ulong window) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.Xlib;

    public nint Display { get; } = display;
    public ulong Window { get; } = window;
}

internal class MetalLayerChainSurface(nint layer) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.MetalLayer;

    /// <summary>
    /// Instance of CAMetalLayer.
    /// </summary>
    public nint Layer { get; } = layer;
}

internal class Win32SwapChainSurface(nint hwnd) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.Win32;

    public nint Hwnd { get; } = hwnd;
}

internal class SwapChainPanelChainSurface(nint swapChainPanel) : SurfaceSource
{
    /// <inheritdoc />
    public override SurfaceSourceType Kind => SurfaceSourceType.SwapChainPanel;

    public nint SwapChainPanel { get; } = swapChainPanel;
}
