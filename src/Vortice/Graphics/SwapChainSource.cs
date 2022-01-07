// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics;

/// <summary>
/// Defines a platform specific surface used for <see cref="SwapChain"/> creation.
/// </summary>
public abstract class SwapChainSource
{
    protected SwapChainSource()
    {

    }

    public abstract SwapChainSourceType Type { get; }

    /// <summary>
    /// Creates a new <see cref="SurfaceSource"/> for a Win32 window.
    /// </summary>
    /// <param name="hinstance">The Win32 instance handle.</param>
    /// <param name="hwnd">The Win32 window handle.</param>
    /// <returns>A new <see cref="SwapChainSource"/> which can be used to create a <see cref="SwapChain"/> for the given Win32 window.
    /// </returns>
    public static SwapChainSource CreateWin32(IntPtr hinstance, IntPtr hwnd) => new Win32SwapChainSource(hinstance, hwnd);
}

public sealed class Win32SwapChainSource : SwapChainSource
{
    internal Win32SwapChainSource(IntPtr hinstance, IntPtr hwnd)
    {
        Hinstance = hinstance;
        Hwnd = hwnd;
    }

    public IntPtr Hinstance { get; }
    public IntPtr Hwnd { get; }

    /// <inheritdoc />
    public override SwapChainSourceType Type => SwapChainSourceType.Win32;
}
