// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Texture : GraphicsResource
{
    protected Texture(in TextureDescriptor descriptor)
        : base(descriptor.Label)
    {
        Dimension = descriptor.Dimension;
        Format = descriptor.Format;
        Width = descriptor.Width;
        Height = descriptor.Height;
        Depth = (descriptor.Dimension == TextureDimension.Texture3D) ? descriptor.DepthOrArrayLayers : 1;
        ArrayLayers = (descriptor.Dimension != TextureDimension.Texture3D) ? descriptor.DepthOrArrayLayers : 1;
        MipLevelCount = descriptor.MipLevelCount;
        SampleCount = descriptor.SampleCount;
        Usage = descriptor.Usage;
        CpuAccess = descriptor.CpuAccess;
    }

    /// <summary>
    /// Gets the texture dimension.
    /// </summary>
    public TextureDimension Dimension { get; }

    /// <summary>
    /// Gets the texture format.
    /// </summary>
    public PixelFormat Format { get; }

    /// <summary>
    /// Gets the texture width, in texels.
    /// </summary>
    public uint Width { get; }

    /// <summary>
    /// Gets the texture height, in texels.
    /// </summary>
    public uint Height { get; }

    /// <summary>
    /// Gets the texture depth, in texels.
    /// </summary>
    public uint Depth { get; }

    /// <summary>
    /// Gets the texture total number of array layers.
    /// </summary>
    public uint ArrayLayers { get; }

    /// <summary>
    /// Gets the total number of mipmap levels in this texture.
    /// </summary>
    public uint MipLevelCount { get; }

    /// <summary>
    /// Gets the texture <see cref="TextureUsage"/>.
    /// </summary>
    public TextureUsage Usage { get; }

    /// <summary>
    /// Gets the texture sample count.
    /// </summary>
    public TextureSampleCount SampleCount { get; }

    /// <summary>
    /// Gets the CPU access of the texure.
    /// </summary>
    public CpuAccessMode CpuAccess { get; }

    /// <summary>
    /// Get a mip-level width.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetWidth(int mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Width >> mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level height.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetHeight(int mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Height >> mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level depth.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetDepth(int mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Depth >> mipLevel) : 0;
    }

    public uint CalculateSubresource(uint mipLevel, uint arrayLayer, uint planeSlice = 0)
    {
        return mipLevel + arrayLayer * MipLevelCount + planeSlice * MipLevelCount * ArrayLayers;
    }

    public static Texture FromFile(GraphicsDevice device, string filePath, int channels = 4, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(device, stream, channels, srgb);
    }

    public static Texture FromStream(GraphicsDevice device, Stream stream, int channels = 4, bool srgb = true)
    {
        Span<byte> data = stackalloc byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(device, data, channels, srgb);
    }

    public static Texture FromMemory(GraphicsDevice device, Span<byte> data, int channels = 4, bool srgb = true)
    {
        using Image image = Image.FromMemory(data, channels, srgb);
        return device.CreateTexture2D(image.Data, image.Format, (uint)image.Width, (uint)image.Height);
    }
}
