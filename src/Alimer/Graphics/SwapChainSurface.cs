// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if WINDOWS
using Microsoft.UI.Xaml.Controls;
using Windows.UI.Core;
#endif

namespace Alimer.Graphics;

/// <summary>
/// Defines a platform specific surface used for <see cref="SwapChain"/> creation.
/// </summary>
public abstract class SwapChainSurface
{
    protected SwapChainSurface()
    {

    }

    public abstract SwapChainSurfaceType Kind { get; }

    /// <summary>
    /// Creates a new <see cref="SwapChainSurface"/> for a Win32 window.
    /// </summary>
    /// <param name="hwnd">The Win32 window handle.</param>
    /// <returns>A new <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given Win32 window.
    /// </returns>
    public static SwapChainSurface CreateWin32(IntPtr hwnd) => new Win32SwapChainSurface(hwnd);

    /// <summary>
    /// Creates a new <see cref="SwapChainSurface"/> for a Android NativeWindow (ANativeWindow).
    /// </summary>
    /// <param name="hwnd">The Android window.</param>
    /// <returns>A new <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given Android window.
    /// </returns>
    public static SwapChainSurface CreateAndroid(IntPtr window) => new AndroidSwapChainSurface(window);

#if WINDOWS
    /// <summary>
    /// Creates a new <see cref="SwapChainSurface"/> for a <see cref="CoreWindow"/>.
    /// </summary>
    /// <param name="coreWindow">The <see cref="CoreWindow"/> handle.</param>
    /// <returns>A new <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given Win32 window.
    /// </returns>
    public static SwapChainSurface CreateCoreWindow(CoreWindow coreWindow) => new CoreWindowSwapChainSurface(coreWindow);

    /// <summary>
    /// Creates a new <see cref="SwapChainSurface"/> for a <see cref="SwapChainPanel"/>.
    /// </summary>
    /// <param name="panel">The <see cref="SwapChainPanel"/> instance.</param>
    /// <returns>A new <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChain"/> for the given SwapChainPanel.
    /// </returns>
    public static SwapChainSurface CreateSwapChainPanel(SwapChainPanel panel) => new SwapChainPanelSwapChainSurface(panel);
#endif
}

internal sealed class Win32SwapChainSurface : SwapChainSurface
{
    public Win32SwapChainSurface(IntPtr hwnd)
    {
        Hwnd = hwnd;
    }

    public IntPtr Hwnd { get; }

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Win32;
}

internal sealed class AndroidSwapChainSurface : SwapChainSurface
{
    public AndroidSwapChainSurface(IntPtr window)
    {
        Window = window;
    }

    public IntPtr Window { get; }

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.Android;
}

#if WINDOWS
internal class CoreWindowSwapChainSurface : SwapChainSurface
{
    public CoreWindowSwapChainSurface(CoreWindow coreWindow)
    {
        CoreWindow = coreWindow;
    }

    public CoreWindow CoreWindow { get; }

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.CoreWindow;
}

internal class SwapChainPanelSwapChainSurface : SwapChainSurface
{
    public SwapChainPanel Panel { get; }

    public SwapChainPanelSwapChainSurface(SwapChainPanel panel)
    {
        Panel = panel;
    }

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind => SwapChainSurfaceType.SwapChainPanel;
}
#endif
