// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes a single mipmap in <see cref="Image"/>.
/// </summary>
public record struct MipMapDescription
{
    [SetsRequiredMembers]
    public MipMapDescription(uint width,
        uint height,
        uint depth,
        uint rowPitch,
        uint slicePitch,
        uint widthPacked, uint heightPacked)
    {
        Guard.IsGreaterThanOrEqualTo(width, 1);
        Guard.IsGreaterThanOrEqualTo(height, 1);
        Guard.IsGreaterThanOrEqualTo(depth, 1);

        Width = width;
        Height = height;
        Depth = depth;
        RowPitch = rowPitch;
        SlicePitch = slicePitch;
        WidthPacked = widthPacked;
        HeightPacked = heightPacked;
        SizeInBytes = SlicePitch * depth;
    }

    /// <summary>
    /// Gets the width of this mipmap.
    /// </summary>
    public required uint Width;

    /// <summary>
    /// Gets the height of this mipmap.
    /// </summary>
    public required uint Height;

    /// <summary>
    /// Gets the depth of this mipmap.
    /// </summary>
    public required uint Depth;

    /// <summary>
    /// Gets the row pitch of this mipmap (number of bytes per row).
    /// </summary>
    public required uint RowPitch;

    /// <summary>
    /// Gets the slice pitch of this mipmap (number of bytes per depth slice).
    /// </summary>
    public required uint SlicePitch;

    /// <summary>
    /// Gets the width of this mipmap.
    /// </summary>
    public required uint WidthPacked;

    /// <summary>
    /// Gets the height of this mipmap.
    /// </summary>
    public readonly uint HeightPacked;

    /// <summary>
    /// Get the size in bytes of this mipmap.
    /// </summary>
    public uint SizeInBytes { get; }
}
