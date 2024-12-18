// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Image"/>.
/// </summary>
public readonly record struct ImageDescription
{
    [SetsRequiredMembers]
    public ImageDescription(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1)
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
        MipLevelCount = mipLevelCount == 0 ? GetMipLevelCount(width, height, dimension == TextureDimension.Texture3D ? depthOrArrayLayers : 1) : mipLevelCount;
    }

    public static ImageDescription Image1D(
        PixelFormat format,
        uint width,
        uint mipLevelCount = 1,
        uint arrayLayers = 1)
    {
        return new(
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
        uint mipLevelCount = 1,
        uint arrayLayers = 1)
    {
        return new(
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
        uint depth,
        uint mipLevelCount = 1)
    {
        return new(
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
        uint mipLevelCount = 1,
        uint arrayLayers = 1)
    {
        return new(
            TextureDimension.TextureCube,
            format,
            size,
            size,
            arrayLayers * 6,
            mipLevelCount);
    }

    /// <summary>
    /// Gets or the dimension of <see cref="Image"/>
    /// </summary>
    public required TextureDimension Dimension { get; init; }

    /// <summary>
    /// Gets the pixel format of <see cref="Image"/>
    /// </summary>
    public required PixelFormat Format { get; init; }

    /// <summary>
    /// Gets the width of <see cref="Image"/>
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

    public static uint GetMipLevelCount(uint width, uint height, uint depth = 1, uint minDimension = 1, uint requiredAlignment = 1u)
    {
        uint mipLevelCount = 1;
        while (width > minDimension || height > minDimension || depth > minDimension)
        {
            width = Math.Max(minDimension, width >> 1);
            height = Math.Max(minDimension, height >> 1);
            depth = Math.Max(minDimension, depth >> 1);
            if (MathHelper.AlignUp(width, requiredAlignment) != width
                || MathHelper.AlignUp(height, requiredAlignment) != height
                || MathHelper.AlignUp(depth, requiredAlignment) != depth)
            {
                break;
            }

            mipLevelCount++;
        }

        return mipLevelCount;
    }
}
