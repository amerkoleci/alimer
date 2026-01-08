// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Texture"/> asset.
/// </summary>
public class TextureAsset : AssetWithSource
{
    /// <summary>
    /// Gets or sets the dimension of <see cref="Texture"/>
    /// </summary>
    public TextureDimension Dimension { get; set; } = TextureDimension.Texture2D;

    /// <summary>
    /// Gets or sets the pixel format of <see cref="TextureAsset"/>
    /// </summary>
    public PixelFormat Format { get; set; } = PixelFormat.RGBA8Unorm;

    public required int Width { get; set; }
    public int Height { get; set; }
    public int DepthOrArrayLayers { get; set; } = 1;
    public int MipLevels { get; set; } = 1;
    public byte[]? Data { get; set; }

    [SetsRequiredMembers]
    public TextureAsset(int width, int height, PixelFormat format, Span<byte> data)
    {
        Width = width;
        Height = height;
        Format = format;
        Data = data.ToArray();
        //BytesPerPixel = format.GetBitsPerPixel() / 8;
    }

    public static TextureAsset Create<T>(int width, int height, PixelFormat format, Span<T> data)
        where T : unmanaged
    {
        return new(width, height, format, MemoryMarshal.Cast<T, byte>(data));
    }

    public Texture CreateRuntime(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));
        Guard.IsNotNull(Data, nameof(Data));

        return device.CreateTexture2D(Data!, Format, (uint)Width, (uint)Height);
    }
}
