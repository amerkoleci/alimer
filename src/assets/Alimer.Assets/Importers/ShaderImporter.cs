// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SkiaSharp;
using Alimer.Graphics;
using Alimer.Assets.Graphics;

namespace Alimer.Assets;

/// <summary>
/// Defines a <see cref="Shader"/> asset importer.
/// </summary>
public sealed class ShaderImporter : AssetImporter<ShaderAsset, ShaderMetadata>
{
    public override Task<ShaderAsset> Import(ShaderMetadata metadata)
    {
        using Stream stream = File.Open(metadata.FileFullPath, FileMode.Open, FileAccess.Read);

        ShaderAsset asset = new()
        {
            Source = metadata.FileFullPath
        };

        return Task.FromResult(asset);
    }
}
