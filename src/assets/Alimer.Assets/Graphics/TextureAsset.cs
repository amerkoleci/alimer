// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

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

    public int Width { get; set; }
    public int Height { get; set; }
    public byte[]? PixelData { get; set; }

    public Texture CreateRuntime(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));
        Guard.IsNotNull(PixelData, nameof(PixelData));

        ReadOnlySpan<byte> span = PixelData!.AsSpan();
        return device.CreateTexture2D(span, Format, (uint)Width, (uint)Height);
    }
}
