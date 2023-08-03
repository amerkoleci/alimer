// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Image"/>.
/// </summary>
public record struct ImageDescription
{
    [SetsRequiredMembers]
    public ImageDescription(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1u)
    {
        Guard.IsTrue(format != PixelFormat.Undefined);
        Guard.IsGreaterThanOrEqualTo(width, 1);
        Guard.IsGreaterThanOrEqualTo(height, 1);
        Guard.IsGreaterThanOrEqualTo(depthOrArrayLayers, 1);

        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArrayLayers = depthOrArrayLayers;
        MipLevelCount = mipLevelCount == 0 ? GraphicsUtilities.GetMipLevelCount(width, height, dimension == TextureDimension.Texture3D ? depthOrArrayLayers : 1) : mipLevelCount;
    }

    public static ImageDescription Image1D(
        PixelFormat format,
        uint width,
        uint mipLevelCount = 1,
        uint arrayLayers = 1u)
    {
        return new ImageDescription(
            TextureDimension.Texture1D,
            format,
            width,
            1,
            arrayLayers,
            mipLevelCount);
    }

    public static ImageDescription Image2D(
        PixelFormat format,
        uint width,
        uint height,
        uint mipLevelCount = 1u,
        uint arrayLayers = 1u)
    {
        return new ImageDescription(
            TextureDimension.Texture2D,
            format,
            width,
            height,
            arrayLayers,
            mipLevelCount);
    }

    public static ImageDescription Image3D(
        PixelFormat format,
        uint width,
        uint height,
        uint depth = 1u,
        uint mipLevelCount = 1u)
    {
        return new ImageDescription(
            TextureDimension.Texture3D,
            format,
            width,
            height,
            depth,
            mipLevelCount);
    }

    public static ImageDescription ImageCube(
        PixelFormat format,
        uint size,
        uint mipLevelCount = 1u,
        uint arrayLayers = 1u)
    {
        return new ImageDescription(
            TextureDimension.Texture2D,
            format,
            size,
            size,
            arrayLayers * 6,
            mipLevelCount);
    }

    /// <summary>
    /// Gets or the dimension of <see cref="Texture"/>
    /// </summary>
    public required TextureDimension Dimension { get; init; }

    /// <summary>
    /// Gets the pixel format of <see cref="Image"/>
    /// </summary>
    public required PixelFormat Format { get; init; }

    /// <summary>
    /// Gets the width of <see cref="Texture"/>
    /// </summary>
    public required uint Width { get; init; } = 1;

    /// <summary>
    /// Gets the height of <see cref="Image"/>
    /// </summary>
    public required uint Height { get; init; } = 1;

    /// <summary>
    /// Gets the depth of <see cref="Image"/>, if it is 3D, or the array layers if it is an array of 1D or 2D resources.
    /// </summary>
    public required uint DepthOrArrayLayers { get; init; } = 1;

    /// <summary>
    /// The number of mipmap levels in the <see cref="Image"/>.
    /// </summary>
    public required uint MipLevelCount { get; init; } = 1;
}
