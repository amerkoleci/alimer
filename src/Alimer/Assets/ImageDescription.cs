// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Assets;

/// <summary>
/// Structure that describes the <see cref="Image"/>.
/// </summary>
public record struct ImageDescription
{
    /// <summary>
    /// Gets or the dimension of <see cref="Image"/>
    /// </summary>
    public TextureDimension Dimension = TextureDimension.Texture2D;

    /// <summary>
    /// Gets the pixel format of <see cref="Image"/>
    /// </summary>
    public PixelFormat Format = PixelFormat.RGBA8Unorm;

    /// <summary>
    /// Gets the width of <see cref="Image"/>
    /// </summary>
    public uint Width = 1;

    /// <summary>
    /// Gets the height of <see cref="Image"/>
    /// </summary>
    public uint Height = 1;

    /// <summary>
    /// Gets the depth of <see cref="Image"/>, if it is 3D, or the array layers if it is an array of 1D or 2D resources.
    /// </summary>
    public uint DepthOrArrayLayers = 1;

    /// <summary>
    /// The number of mipmap levels in the <see cref="Image"/>.
    /// </summary>
    public uint MipLevelCount = 1;

    public ImageDescription(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1)
    {
        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArrayLayers = depthOrArrayLayers;
        MipLevelCount = mipLevelCount;
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
            mipLevelCount
            );
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
            mipLevelCount
            );
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
            mipLevelCount
            );
    }

#if TODO
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
#endif

    public static uint GetMipLevelCount(uint width, uint height, uint depth = 1, uint minDimension = 1, uint requiredAlignment = 1u)
    {
        uint mipLevelCount = 1;
        while (width > minDimension || height > minDimension || depth > minDimension)
        {
            width = Math.Max(minDimension, width >> 1);
            height = Math.Max(minDimension, height >> 1);
            depth = Math.Max(minDimension, depth >> 1);
            if (MathUtilities.AlignUp(width, requiredAlignment) != width
                || MathUtilities.AlignUp(height, requiredAlignment) != height
                || MathUtilities.AlignUp(depth, requiredAlignment) != depth)
            {
                break;
            }

            mipLevelCount++;
        }

        return mipLevelCount;
    }
}
