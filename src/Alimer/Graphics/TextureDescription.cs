// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Texture"/>.
/// </summary>
public record struct TextureDescription
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
    /// CPU access of the <see cref="Texture"/>.
    /// </summary>
    public CpuAccessMode CpuAccess = CpuAccessMode.None;

    /// <summary>
    /// Gets or sets the label of <see cref="Texture"/>.
    /// </summary>
    public string? Label;

    public TextureDescription(
        TextureDimension dimension,
        PixelFormat format,
        uint width,
        uint height,
        uint depthOrArrayLayers,
        uint mipLevelCount = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        CpuAccessMode access = CpuAccessMode.None)
    {
        Dimension = dimension;
        Format = format;
        Width = width;
        Height = height;
        DepthOrArrayLayers = depthOrArrayLayers;
        MipLevelCount = mipLevelCount;
        SampleCount = sampleCount;
        Usage = usage;
        CpuAccess = access;
    }

    public static TextureDescription Texture1D(
        PixelFormat format,
        uint width,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None)
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
            access);
    }

    public static TextureDescription Texture2D(
        PixelFormat format,
        uint width,
        uint height,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        TextureSampleCount sampleCount = TextureSampleCount.Count1,
        CpuAccessMode access = CpuAccessMode.None)
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
            access);
    }

    public static TextureDescription Texture3D(
        PixelFormat format,
        uint width,
        uint height,
        uint depth,
        uint mipLevelCount = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None)
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
            access);
    }

    public static TextureDescription TextureCube(
        PixelFormat format,
        uint size,
        uint mipLevelCount = 1,
        uint arrayLayers = 1,
        TextureUsage usage = TextureUsage.ShaderRead,
        CpuAccessMode access = CpuAccessMode.None)
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
            access);
    }
}
