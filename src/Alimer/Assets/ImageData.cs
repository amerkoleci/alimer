// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using Alimer.Graphics;

namespace Alimer.Assets;

/// <summary>
/// Defines data for single mip level in give <see cref="Image"/>.
/// </summary>
public readonly record struct ImageData
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ImageData"/> struct.
    /// </summary>
    /// <param name="width">The width.</param>
    /// <param name="height">The height.</param>
    /// <param name="format">The format.</param>
    /// <param name="rowPitch">The row pitch.</param>
    /// <param name="slicePitch">The slice pitch.</param>
    /// <param name="dataPointer">The data pointer.</param>
    [SetsRequiredMembers]
    public ImageData(uint width, uint height, PixelFormat format, uint rowPitch, uint slicePitch, nint dataPointer)
    {
        Width = width;
        Height = height;
        Format = format;
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
        DataPointer = dataPointer;
    }

    /// <summary>
    /// Gets the width.
    /// </summary>
    /// <value>The width.</value>
    public uint Width { get; }

    /// <summary>
    /// Gets the height.
    /// </summary>
    /// <value>The height.</value>
    public required uint Height { get; init; }

    /// <summary>
    /// Gets the format.
    /// </summary>
    public required PixelFormat Format { get; init; }

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
