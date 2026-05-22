// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Surface"/>.
/// </summary>
public record struct SurfaceDescriptor
{
    /// <summary>
    /// Gets or sets the source of the <see cref="Surface"/>.
    /// </summary>
    public required SurfaceSource Source;

    /// <summary>
    /// Gets or sets the label of the <see cref="Surface"/> for debugging purposes.
    /// </summary>
    public string? Label = default;

    [SetsRequiredMembers]
    public SurfaceDescriptor(
        SurfaceSource source,
        string? label=default)
    {
        ArgumentNullException.ThrowIfNull(source, nameof(source));

        Source = source;
        Label = label;
    }
}
