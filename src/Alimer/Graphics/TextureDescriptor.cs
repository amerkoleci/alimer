// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Texture"/>.
/// </summary>
public record struct TextureDescriptor
{
    /// <summary>
    /// Gets the dimension of <see cref="Texture"/>
    /// </summary>
    public TextureDimension Dimension = TextureDimension.Texture2D;

    /// <summary>
    /// Gets the pixel format of <see cref="Texture"/>
    /// </summary>
    public PixelFormat Format = PixelFormat.RGBA8Unorm;

    /// <summary>
    /// Gets the width of <see cref="Texture"/>
    /// </summary>
    public uint Width = 1;

    /// <summary>
    /// Gets the height of <see cref="Texture"/>
    /// </summary>
    public uint Height = 1;

    /// <summary>
    /// Gets the depth of <see cref="Texture"/>, if it is 3D, or the array layers if it is an array of 1D or 2D resources.
    /// </summary>
    public uint DepthOrArrayLayers = 1;

    /// <summary>
    /// The number of mipmap levels in the <see cref="Texture"/>.
    /// </summary>
    public uint MipLevelCount = 1;

    /// <summary>
    /// Gets the <see cref="TextureUsage"/> of <see cref="Texture"/>.
    /// </summary>
    public TextureUsage Usage = TextureUsage.ShaderRead;

    /// <summary>
    /// Gets the texture sample count.
    /// </summary>
    public TextureSampleCount SampleCount = TextureSampleCount.Count1;

    /// <summary>
    /// Gets or sets the memory type of the texture.
    /// </summary>
    public MemoryType MemoryType = MemoryType.Private;

    /// <summary>
    /// Gets or sets the label of <see cref="Texture"/>.
    /// </summary>
    public string? Label;

    public TextureDescriptor(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        string? label = default)
    {
        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArrayLayers = depthOrArrayLayers;
        MipLevelCount = mipLevelCount;
        SampleCount = sampleCount;
        Usage = usage;
        Label = label;
    }

    public static TextureDescriptor Texture1D(
        PixelFormat format,
        uint width,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        string? label = default)
    {
        return new(
            TextureDimension.Texture1D,
            format,
            width,
            1,
            arrayLayers,
            mipLevelCount,
            usage,
            TextureSampleCount.Count1,
            label
            );
    }

    public static TextureDescriptor Texture2D(
        PixelFormat format,
        uint width,
        uint height,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        string? label = default)
    {
        return new(
            TextureDimension.Texture2D,
            format,
            width,
            height,
            arrayLayers,
            mipLevelCount,
            usage,
            sampleCount,
            label);
    }

    public static TextureDescriptor Texture3D(
        PixelFormat format,
        uint width,
        uint height,
        uint depth,
        uint mipLevelCount = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        string? label = default)
    {
        return new(
            TextureDimension.Texture3D,
            format,
            width,
            height,
            depth,
            mipLevelCount,
            usage,
            TextureSampleCount.Count1,
            label
            );
    }

    public static TextureDescriptor TextureCube(
        PixelFormat format,
        uint size,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        string? label = default)
    {
        return new(
            TextureDimension.Texture2D,
            format,
            size,
            size,
            arrayLayers * 6,
            mipLevelCount,
            usage,
            TextureSampleCount.Count1,
            label
            );
    }
}
