// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics;

public abstract class Texture : GraphicsObject, IGraphicsBindableResource
{
    protected readonly TextureLayout[] _subresourceLayouts;
    private TextureView? _defaultView;
    protected readonly Dictionary<TextureViewDescriptor, TextureView> _views = [];

    protected Texture(in TextureDescriptor descriptor)
        : base(descriptor.Label)
    {
        Dimension = descriptor.Dimension;
        Format = descriptor.Format;
        Width = descriptor.Width;
        Height = descriptor.Height;
        Depth = (descriptor.Dimension == TextureDimension.Texture3D) ? descriptor.DepthOrArrayLayers : 1;
        ArrayLayers = (descriptor.Dimension != TextureDimension.Texture3D) ? descriptor.DepthOrArrayLayers : 1;
        MipLevelCount = descriptor.MipLevelCount == 0 ? ImageDescription.GetMipLevelCount(Width, Height, Dimension == TextureDimension.Texture3D ? Depth : 1) : descriptor.MipLevelCount;
        SampleCount = descriptor.SampleCount;
        Usage = descriptor.Usage;
        MemoryType = descriptor.MemoryType;

        uint numSubResources = MipLevelCount * descriptor.DepthOrArrayLayers;
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
    /// Gets the memory type of the texure.
    /// </summary>
    public MemoryType MemoryType { get; }

    /// <summary>
    /// Gets a value indicating whether the texture is multisampled.
    /// </summary>
    public bool IsMultisampled => SampleCount > TextureSampleCount.Count1;

    #region View
    public TextureView? DefaultView
    {
        get
        {
            if ((Usage & (TextureUsage.ShaderRead | TextureUsage.ShaderWrite | TextureUsage.RenderTarget)) == 0)
                return default;

            if (_defaultView is not null)
                return _defaultView;

            _defaultView = GetView(new());
            return _defaultView;
        }
    }

    public TextureView GetView() => GetView(default);
    public TextureView GetView(in TextureViewDescriptor descriptor)
    {
        if ((Usage & (TextureUsage.ShaderRead | TextureUsage.ShaderWrite | TextureUsage.RenderTarget)) == 0)
        {
            throw new GraphicsException("Cannot create TextureView for texture without ShaderRead, ShaderWrite or RenderTarget usage");
        }

        if (_views.TryGetValue(descriptor, out TextureView? view))
        {
            return view;
        }

        TextureViewDescriptor creationDesc = descriptor;

        if (creationDesc.Dimension == TextureViewDimension.Undefined)
        {
            creationDesc.Dimension = Dimension switch
            {
                TextureDimension.Texture1D => (ArrayLayers > 1) ? TextureViewDimension.View1DArray : TextureViewDimension.View1D,
                TextureDimension.Texture2D => (ArrayLayers > 1) ? TextureViewDimension.View2DArray : TextureViewDimension.View2D,
                TextureDimension.Texture3D => TextureViewDimension.View3D,
                //case TextureDimension.Cube:
                //    creationDesc.Dimension = (ArrayLayers > 6) ? TextureViewDimension.TextureCubeArray : TextureViewDimension.TextureCube;
                //    break;
                _ => throw new ArgumentOutOfRangeException(nameof(descriptor.Dimension)),
            };
        }

        if (creationDesc.Format == PixelFormat.Undefined)
        {
            creationDesc.Format = Format;
        }

        creationDesc.BaseMipLevel = Math.Min(creationDesc.BaseMipLevel, MipLevelCount);
        if (creationDesc.MipLevelCount == MipLevelCountUndefined)
        {
            creationDesc.MipLevelCount = MipLevelCount - creationDesc.BaseMipLevel;
        }
        else
        {
            creationDesc.MipLevelCount = Math.Min(creationDesc.MipLevelCount, MipLevelCount - creationDesc.BaseMipLevel);
        }

        if (creationDesc.ArrayLayerCount == ArrayLayerCountUndefined)
        {
            switch (creationDesc.Dimension)
            {
                case TextureViewDimension.View1D:
                case TextureViewDimension.View2D:
                case TextureViewDimension.View3D:
                    creationDesc.ArrayLayerCount = 1;
                    break;
                case TextureViewDimension.ViewCube:
                    creationDesc.ArrayLayerCount = 6;
                    break;
                case TextureViewDimension.View1DArray:
                case TextureViewDimension.View2DArray:
                case TextureViewDimension.ViewCubeArray:
                    creationDesc.ArrayLayerCount = ArrayLayers - creationDesc.BaseArrayLayer;
                    break;
                default:
                    break;
            }
        }

        // Multisampled texture view creation only 2D and 2D array textures are supported.
        if (SampleCount > TextureSampleCount.Count1)
        {
            if (creationDesc.Dimension != TextureViewDimension.View2D &&
                creationDesc.Dimension != TextureViewDimension.View2DArray)
            {
                throw new GraphicsException("Multisampled texture views are only supported for 2D and 2D array textures.");
            }
        }

        //uint textureArrayLayerCount = GetArrayLayers() * (type == TextureType::TextureCube ? 6 : 1);
        //creationDesc.BaseArrayLayer = MatMin(creationDesc.baseArrayLayer, textureArrayLayerCount);
        //creationDesc.ArrayLayerCount = Min(creationDesc.arrayLayerCount, textureArrayLayerCount - creationDesc.baseArrayLayer);

        view = CreateView(in creationDesc);
        _views[descriptor] = view;
        return view;
    }

    protected abstract TextureView CreateView(in TextureViewDescriptor descriptor);
    protected void DestroyViews()
    {
        foreach (TextureView view in _views.Values)
        {
            view.Destroy();
        }
        _views.Clear();
    }
    #endregion

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

    public uint GetSubresourceIndex(uint mipLevel, uint arrayLayer, uint planeSlice = 0)
    {
        return mipLevel + arrayLayer * MipLevelCount + planeSlice * MipLevelCount * ArrayLayers;
    }

    public TextureLayout GetTextureLayout(uint mipLevel, uint arrayLayer, uint placeSlice = 0)
    {
        Debug.Assert(mipLevel < MipLevelCount);
        Debug.Assert(arrayLayer < Depth * ArrayLayers);

        uint subresource = GetSubresourceIndex(mipLevel, arrayLayer, placeSlice);
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

        uint subresource = GetSubresourceIndex(mipLevel, arrayLayer, placeSlice);
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
                uint subresource = GetSubresourceIndex(mipLevel, arrayLayer);
                _subresourceLayouts[subresource] = newLayout;
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
