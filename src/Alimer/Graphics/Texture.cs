// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics.VGPU;
using static Alimer.Graphics.VGPU.VGPU;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics;

public sealed class Texture : GraphicsResource
{
    internal VGPUTexture Handle { get; }

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

    public unsafe Texture(GraphicsDevice device, in TextureDescription description)
        : base(device, true, description.Label)
    {
        Dimension = description.Dimension;
        Format = description.Format;
        Width = description.Width;
        Height = description.Height;
        Depth = (description.Dimension == TextureDimension.Texture3D) ? description.DepthOrArrayLayers : 1;
        ArrayLayers = (description.Dimension != TextureDimension.Texture3D) ? description.DepthOrArrayLayers : 1;
        MipLevelCount = description.MipLevelCount;
        SampleCount = description.SampleCount;
        Usage = description.Usage;
        CpuAccess = description.CpuAccess;

        fixed (sbyte* pLabel = description.Label.GetUtf8Span())
        {
            VGPUTextureDesc nativeDesc = new()
            {
                label = pLabel,
                width = description.Width,
                height = description.Height,
                format = (VGPUTextureFormat)description.Format,
            };
            Handle = vgpuCreateTexture(device.Handle, &nativeDesc, null);
        }
    }

    internal Texture(GraphicsDevice device, in VGPUTexture handle)
        : base(device, false)
    {
        Handle = handle;
    }

    /// <summary>
    /// Get a mip-level width.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetWidth(int mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Width >> mipLevel) : 0;
    }

    // <summary>
    /// Get a mip-level height.
    /// </summary>
    /// <param name="mipLevel"></param>
    /// <returns></returns>
    public uint GetHeight(int mipLevel = 0)
    {
        return (mipLevel == 0) || (mipLevel < MipLevelCount) ? Math.Max(1, Height >> mipLevel) : 0;
    }

    // <summary>
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

    /// <summary>
    /// Finalizes an instance of the <see cref="Texture" /> class.
    /// </summary>
    ~Texture() => Dispose(disposing: false);

    protected override void Destroy()
    {
        _ = vgpuTextureRelease(Handle);
    }
}
