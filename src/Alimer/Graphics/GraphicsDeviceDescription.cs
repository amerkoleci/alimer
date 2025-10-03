// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="GraphicsDevice"/>.
/// </summary>
public record struct GraphicsDeviceDescription
{
    public GraphicsDeviceDescription()
    {
    }

    /// <summary>
    /// Gets or sets the maximum number of frames that can be processed concurrently.
    /// </summary>
    public uint MaxFramesInFlight { get; set; } = Constants.DefaultMaxFramesInFlight;

    /// <summary>
    /// Gets or sets the label of <see cref="GraphicsDevice"/>.
    /// </summary>
    public string? Label { get; set; } = default;
}
