// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SkiaSharp;
using Alimer.Graphics;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="Texture"/> asset importer.
/// </summary>
public sealed class TextureImporter : AssetImporter<TextureAsset>
{
    public override Task<TextureAsset> Import(string source, IServiceProvider services)
    {
        //using Stream stream = await contentManager.FileProvider.OpenStreamAsync(Source, FileMode.Open, FileAccess.Read);
        using Stream stream = File.Open(source, FileMode.Open, FileAccess.Read);
        using SKBitmap bitmap = SKBitmap.Decode(stream);

        TextureAsset asset = new()
        {
            Source = source
        };

        return Task.FromResult(asset);
    }
}
