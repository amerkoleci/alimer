// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Defines data for single mip level in give <see cref="Image"/>.
/// </summary>
public readonly struct ImageData
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ImageData"/> struct.
    /// </summary>
    /// <param name="rowPitch">The row pitch.</param>
    /// <param name="slicePitch">The slice pitch.</param>
    /// <param name="dataPointer">The data pointer.</param>
    [SetsRequiredMembers]
    public ImageData(uint rowPitch, uint slicePitch, nint dataPointer)
    {
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
        DataPointer = dataPointer;
    }

    /// <summary>
    /// Gets the number of bytes per row.
    /// </summary>
    public required uint RowPitch { get; init; }

    /// <summary>
    /// Gets the number of bytes per slice (for a 3D texture, a slice is a 2D image)
    /// </summary>
    public required uint SlicePitch { get; init; }

    /// <summary>
    /// Pointer to the data.
    /// </summary>
    public required nint DataPointer { get; init; }
}
