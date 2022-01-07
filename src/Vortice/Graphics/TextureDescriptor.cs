// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics;

/// <summary>
/// Structure that describes the <see cref="Texture"/>.
/// </summary>
public record struct TextureDescriptor : IEquatable<TextureDescriptor>
{
    public TextureDescriptor(
        TextureDimension dimension,
        TextureFormat format,
        int width,
        int height,
        int depthOrArraySize,
        int mipLevels = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1)
    {
        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArraySize = depthOrArraySize;
        MipLevels = mipLevels == 1 ? CountMipLevels(width, height, dimension == TextureDimension.Texture3D ? depthOrArraySize : 1) : mipLevels;
        SampleCount = sampleCount;
        Usage = usage;
    }

    public static TextureDescriptor Texture1D(
        TextureFormat format,
        int width,
        int mipLevels = 1,
        int arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead)
    {
        return new TextureDescriptor(
            TextureDimension.Texture1D,
            format,
            width,
            1,
            arrayLayers,
            mipLevels,
            usage,
            TextureSampleCount.Count1);
    }

    public static TextureDescriptor Texture2D(
        TextureFormat format,
        int width,
        int height,
        int mipLevels = 1,
        int arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1
        )
    {
        return new TextureDescriptor(
            TextureDimension.Texture2D,
            format,
            width,
            height,
            arrayLayers,
            mipLevels,
            usage,
            sampleCount);
    }

    public static TextureDescriptor Texture3D(
        TextureFormat format,
        int width,
        int height,
        int depth = 1,
        int mipLevels = 1,
        TextureUsage usage = TextureUsage.ShaderRead)
    {
        return new TextureDescriptor(
            TextureDimension.Texture3D,
            format,
            width,
            height,
            depth,
            mipLevels,
            usage,
            TextureSampleCount.Count1);
    }

    /// <summary>
    /// Dimension of texture.
    /// </summary>
    public TextureDimension Dimension { get; init; }

    public TextureFormat Format { get; init; }

    public int Width { get; init; }
    public int Height { get; init; }
    public int DepthOrArraySize { get; init; }
    public int MipLevels { get; init; }
    public TextureUsage Usage { get; init; }
    public TextureSampleCount SampleCount { get; init; }

    /// <summary>
    /// Returns the number of mip levels given a texture size
    /// </summary>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="depth"></param>
    /// <returns></returns>
    private static int CountMipLevels(int width, int height, int depth = 1)
    {
        int numMips = 0;
        int size = Math.Max(Math.Max(width, height), depth);
        while (1 << numMips <= size)
        {
            ++numMips;
        }

        if (1 << numMips < size)
            ++numMips;

        return numMips;
    }
}
