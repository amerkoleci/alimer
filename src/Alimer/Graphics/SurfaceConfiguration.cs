// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Surface"/>.
/// </summary>
public record struct SurfaceConfiguration
{
    /// <summary>
    /// Gets or sets the <see cref="GraphicsDevice"/> associated with the <see cref="Surface"/>.
    /// </summary>
    public required GraphicsDevice Device;
    /// <summary>
    /// Gets or sets the width in pixels of the <see cref="Surface"/>.
    /// </summary>
    public required int Width;
    /// <summary>
    /// Gets or sets the height in pixels of the <see cref="Surface"/>.
    /// </summary>
    public required int Height;

    public PixelFormat Format = PixelFormat.BGRA8UnormSrgb;
    public CompositeAlphaMode AlphaMode = CompositeAlphaMode.Auto;
    public PresentMode PresentMode = PresentMode.Fifo;
    public string? Label = default;

    [SetsRequiredMembers]
    public SurfaceConfiguration(
        GraphicsDevice device,
        int width, int height,
        PixelFormat colorFormat = PixelFormat.BGRA8UnormSrgb,
        CompositeAlphaMode alphaMode = CompositeAlphaMode.Auto,
        PresentMode presentMode = PresentMode.Fifo)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(width, nameof(width));
        ArgumentOutOfRangeException.ThrowIfNegativeOrZero(height, nameof(height));

        Device = device;
        Width = Math.Max(width, 1);
        Height = Math.Max(height, 1);
        Format = colorFormat;
        AlphaMode = alphaMode;
        PresentMode = presentMode;
    }
}
