// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Graphics;

public abstract class Texture : GraphicsResource
{
    protected readonly TextureLayout[] _subresourceLayouts;

    protected Texture(in TextureDescription description)
        : base(description.Label)
    {
        Dimension = description.Dimension;
        Format = description.Format;
        Width = description.Width;
        Height = description.Height;
        Depth = (description.Dimension == TextureDimension.Texture3D) ? description.DepthOrArrayLayers : 1u;
        ArrayLayers = (description.Dimension != TextureDimension.Texture3D) ? description.DepthOrArrayLayers : 1u;
        MipLevelCount = Math.Max(description.MipLevelCount, 1u);
        SampleCount = description.SampleCount;
        Usage = description.Usage;
        CpuAccess = description.CpuAccess;

        uint numSubResources = MipLevelCount * description.DepthOrArrayLayers;
        _subresourceLayouts = new TextureLayout[numSubResources];
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
    public uint GetWidth(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Width >> (int)mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level height.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetHeight(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Height >> (int)mipLevel) : 0;
    }

    /// <summary>
    /// Get a mip-level depth.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetDepth(uint mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Depth >> (int)mipLevel) : 0;
    }

    public uint CalculateSubresource(uint mipLevel, uint arrayLayer, uint planeSlice = 0)
    {
        return mipLevel + arrayLayer * MipLevelCount + planeSlice * MipLevelCount * ArrayLayers;
    }

    public TextureLayout GetTextureLayout(uint mipLevel, uint arrayLayer, uint placeSlice = 0)
    {
        Debug.Assert(mipLevel < MipLevelCount);
        Debug.Assert(arrayLayer < Depth * ArrayLayers);

        uint subresource = CalculateSubresource(mipLevel, arrayLayer, placeSlice);
        return _subresourceLayouts[subresource];
    }

    public TextureLayout GetTextureLayout(uint subresource)
    {
        Debug.Assert(subresource < _subresourceLayouts.Length);

        return _subresourceLayouts[subresource];
    }

    protected void SetTextureLayout(TextureLayout newLayout)
    {
        for (int i = 0; i < _subresourceLayouts.Length; i++)
        {
            _subresourceLayouts[i] = newLayout;
        }
    }

    internal void SetTextureLayout(uint subresource, TextureLayout newLayout)
    {
        Debug.Assert(subresource < _subresourceLayouts.Length);

        _subresourceLayouts[subresource] = newLayout;
    }

    internal void SetTextureLayout(TextureLayout newLayout, uint mipLevel, uint arrayLayer, uint placeSlice = 0)
    {
        Debug.Assert(mipLevel < MipLevelCount);
        Debug.Assert(arrayLayer < Depth * ArrayLayers);

        uint subresource = CalculateSubresource(mipLevel, arrayLayer, placeSlice);
        _subresourceLayouts[subresource] = newLayout;
    }

    internal void SetTextureLayout(TextureLayout newLayout,
        uint baseMiplevel,
        uint levelCount,
        uint baseArrayLayer,
        uint layerCount)
    {
        for (uint arrayLayer = baseArrayLayer; arrayLayer < (baseArrayLayer + layerCount); arrayLayer++)
        {
            for (uint mipLevel = baseMiplevel; mipLevel < (baseMiplevel + levelCount); mipLevel++)
            {
                uint iterSubresource = CalculateSubresource(mipLevel, arrayLayer);
                _subresourceLayouts[iterSubresource] = newLayout;
            }
        }
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
