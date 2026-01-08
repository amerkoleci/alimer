// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SkiaSharp;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Texture"/> asset importer.
/// </summary>
public sealed class TextureImporter : AssetImporter<TextureAsset, TextureMetadata>
{
    public override Task<TextureAsset> Import(TextureMetadata metadata)
    {
        //using Stream stream = await contentManager.FileProvider.OpenStreamAsync(Source, FileMode.Open, FileAccess.Read);
        using Stream stream = File.Open(metadata.FileFullPath, FileMode.Open, FileAccess.Read);

        using SKBitmap bitmap = SKBitmap.Decode(stream);

        // Convert from BGRA8 to RGBA8
        using SKBitmap newBitmap = new(bitmap.Width, bitmap.Height, SKColorType.Rgba8888, bitmap.AlphaType);
        Guard.IsTrue(bitmap.CopyTo(newBitmap, SKColorType.Rgba8888));

        TextureAsset asset = new(newBitmap.Width, newBitmap.Height, PixelFormat.RGBA8Unorm, newBitmap.Bytes)
        {
            Source = metadata.FileFullPath,
        };

        return Task.FromResult(asset);
    }
}
