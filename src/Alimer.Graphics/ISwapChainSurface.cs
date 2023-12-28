// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Mathematics;

namespace Alimer.Graphics;

/// <summary>
/// Defines a platform specific surface used for <see cref="SwapChain"/>.
/// </summary>
public interface ISwapChainSurface
{
    /// <summary>Occurs when the <see cref="Size" /> property changes.</summary>
    event EventHandler? SizeChanged;

    /// <summary>
    /// Gets the surface kind.
    /// </summary>
    SwapChainSurfaceType Kind { get; }

    /// <summary>Gets a context handle for the surface.</summary>
    /// <exception cref="ObjectDisposedException">The surface has been disposed.</exception>
    nint ContextHandle { get; }

    /// <summary>Gets a handle for the surface.</summary>
    /// <exception cref="ObjectDisposedException">The surface has been disposed.</exception>
    nint Handle { get; }

    /// <summary>
    /// Gets the size, in pixels, of the surface.
    /// </summary>
    SizeI Size { get; }

    /// <summary>
    /// Gets the width, in pixels, of the surface.
    /// </summary>
    float PixelWidth => Size.Width;

    /// <summary>
    /// Gets the height, in pixels, of the surface.
    /// </summary>
    float PixelHeight => Size.Height;
}
