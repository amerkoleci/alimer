// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines a <see cref="SwapChainSurface"/> type.
/// </summary>
public enum SwapChainSurfaceType
{
    /// <summary>Defines an unknown graphics surface kind that may require specialized handling.</summary>
    Unknown,

    /// <summary>Defines an Android based graphics surface.</summary>
    Android,

    /// <summary>Defines a Wayland based graphics surface.</summary>
    Wayland,

    /// <summary>Defines an Xlib based graphics surface.</summary>
    Xlib,

    /// <summary>Defines an MetalLayer based graphics surface.</summary>
    MetalLayer,

    /// <summary>Defines a Win32 based graphics surface.</summary>
    Win32,

    /// <summary>Defines a WinUI SwapChainPanel based graphics surface.</summary>
    SwapChainPanel,
}
