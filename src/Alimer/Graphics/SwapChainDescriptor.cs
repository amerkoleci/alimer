// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="SwapChain"/>.
/// </summary>
public record struct SwapChainDescriptor
{
    /// <summary>
    /// Gets or sets the platform surface of the <see cref="SwapChain"/>.
    /// </summary>
    public required SwapChainSurface Surface;

    /// <summary>
    /// Gets or sets the width in pixels of the <see cref="SwapChain"/>.
    /// </summary>
	public required int Width;
    /// <summary>
    /// Gets or sets the height in pixels of the <see cref="SwapChain"/>.
    /// </summary>
    public required int Height;

    public PixelFormat Format  = PixelFormat.BGRA8UnormSrgb;
    public PresentMode PresentMode = PresentMode.Fifo;
    public string? Label = default;

    [SetsRequiredMembers]
    public SwapChainDescriptor(
        SwapChainSurface surface,
        int width, int height,
        PixelFormat colorFormat = PixelFormat.BGRA8UnormSrgb,
        PresentMode presentMode = PresentMode.Fifo)
    {
        ArgumentNullException.ThrowIfNull(surface);
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(width, nameof(width));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(height, nameof(height));

        Surface = surface;
        Width = width;
        Height = height;
        Format = colorFormat;
        PresentMode = presentMode;
    }
}
