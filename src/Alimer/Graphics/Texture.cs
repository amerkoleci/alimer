// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics;

[AssetReader(typeof(TextureAssetReader))]
public abstract partial class Texture : GraphicsObject, IGraphicsBindableResource
{
    private readonly TextureLayout[] _subresourceLayouts;
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

        ActualArrayLayers = (descriptor.Dimension == TextureDimension.TextureCube) ? 6 * ArrayLayers : ArrayLayers;

        uint numSubResources = MipLevelCount * ActualArrayLayers * Depth;
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
    /// Actual number of array layers (taking care of cubemap textures)
    /// </summary>
    internal uint ActualArrayLayers { get; }

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

            if (field is not null)
                return field;

            field = GetView(new());
            return field;
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

        TextureViewDescriptor creationDescriptor = descriptor;

        if (creationDescriptor.Dimension == TextureViewDimension.Undefined)
        {
            creationDescriptor.Dimension = Dimension switch
            {
                TextureDimension.Texture1D => (ArrayLayers > 1) ? TextureViewDimension.View1DArray : TextureViewDimension.View1D,
                TextureDimension.Texture2D => (ArrayLayers > 1) ? TextureViewDimension.View2DArray : TextureViewDimension.View2D,
                TextureDimension.Texture3D => TextureViewDimension.View3D,
                TextureDimension.TextureCube => (ArrayLayers > 1) ? TextureViewDimension.ViewCubeArray : TextureViewDimension.ViewCube,
                _ => throw new ArgumentOutOfRangeException(nameof(descriptor.Dimension)),
            }
        ;
        }

        if (creationDescriptor.Format == PixelFormat.Undefined)
        {
            creationDescriptor.Format = Format;
        }

        creationDescriptor.BaseMipLevel = Math.Min(creationDescriptor.BaseMipLevel, MipLevelCount);
        if (creationDescriptor.MipLevelCount == MipLevelCountUndefined)
        {
            creationDescriptor.MipLevelCount = MipLevelCount - creationDescriptor.BaseMipLevel;
        }
        else
        {
            creationDescriptor.MipLevelCount = Math.Min(creationDescriptor.MipLevelCount, MipLevelCount - creationDescriptor.BaseMipLevel);
        }

        if (creationDescriptor.ArrayLayerCount == ArrayLayerCountUndefined)
        {
            switch (creationDescriptor.Dimension)
            {
                case TextureViewDimension.View1D:
                case TextureViewDimension.View2D:
                case TextureViewDimension.View3D:
                case TextureViewDimension.ViewCube:
                    creationDescriptor.ArrayLayerCount = 1;
                    break;
                case TextureViewDimension.View1DArray:
                case TextureViewDimension.View2DArray:
                case TextureViewDimension.ViewCubeArray:
                    creationDescriptor.ArrayLayerCount = ArrayLayers - creationDescriptor.BaseArrayLayer;
                    break;
                default:
                    break;
            }
        }

        // Multisampled texture view creation only 2D and 2D array textures are supported.
        if (SampleCount > TextureSampleCount.Count1)
        {
            if (creationDescriptor.Dimension != TextureViewDimension.View2D &&
                creationDescriptor.Dimension != TextureViewDimension.View2DArray)
            {
                throw new GraphicsException("Multisampled texture views are only supported for 2D and 2D array textures.");
            }
        }

        if (creationDescriptor.Dimension == TextureViewDimension.ViewCube
            || creationDescriptor.Dimension == TextureViewDimension.ViewCubeArray)
        {
            if (creationDescriptor.ArrayLayerCount > 1
                && creationDescriptor.ArrayLayerCount % 6 != 0)
            {
                Log.Error($"Cube texture view array layer count must be a multiple of 6, got {creationDescriptor.ArrayLayerCount}");
            }
        }

        view = CreateView(in creationDescriptor);
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

    public static Texture FromFile(GPUDevice device, string filePath, int channels = 4, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(device, stream, channels, srgb);
    }

    public static Texture FromStream(GPUDevice device, Stream stream, int channels = 4, bool srgb = true)
    {
        Span<byte> data = stream.Length < 2048 ? stackalloc byte[(int)stream.Length] : new byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(device, data, channels, srgb);
    }

    public static Texture FromMemory(GPUDevice device, Span<byte> data, int channels = 4, bool srgb = true)
    {
        using Image image = Image.FromMemory(data, channels, srgb);

        TextureDescriptor descriptor = new()
        {
            Label = image.Name,
            Dimension = image.Dimension,
            Format = image.Format,
            Width = image.Width,
            Height = image.Height,
            DepthOrArrayLayers = image.Description.DepthOrArrayLayers,
            MipLevelCount = image.MipLevelCount
        };

        int index = 0;
        Span<TextureData> initData = stackalloc TextureData[(int)(image.ActualArrayLayers * descriptor.MipLevelCount)];
        for (uint arrayIndex = 0; arrayIndex < image.ActualArrayLayers; arrayIndex++)
        {
            for (uint mipLevel = 0; mipLevel < descriptor.MipLevelCount; mipLevel++)
            {
                ImageData level = image.GetLevel(mipLevel, arrayIndex);
                initData[index] = new TextureData(level.DataPointer, level.RowPitch, level.SlicePitch);
                index++;
            }
        }

        return device.CreateTexture(in descriptor, initData);
    }
}
