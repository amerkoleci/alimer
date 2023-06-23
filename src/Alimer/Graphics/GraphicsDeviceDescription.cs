// Copyright © Amer Koleci and Contributors.
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
    /// Gets or sets the preferred backend to creates.
    /// </summary>
    public GraphicsBackendType PreferredBackend { get; set; } = GraphicsBackendType.Count;

    /// <summary>
    /// Gets the <see cref="GraphicsDevice"/> validation mode.
    /// </summary>
    public ValidationMode ValidationMode { get; set; } = ValidationMode.Disabled;

    /// <summary>
    /// Gets the GPU adapter selection power preference.
    /// </summary>
    public GpuPowerPreference PowerPreference { get; set; } = GpuPowerPreference.Undefined;

    // <summary>
    /// Gets or sets the label of <see cref="GraphicsDevice"/>.
    /// </summary>
    public string? Label { get; set; } = default;
}
