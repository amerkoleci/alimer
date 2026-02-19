// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using GLTF2;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MaterialAsset"/> importer.
/// </summary>
public sealed class MaterialImporter : AssetImporter<MaterialAsset, MaterialMetadata>
{
    public Task<MaterialAsset> Import(Gltf2.Material gltfMaterial, string source)
    {
        MaterialAsset asset = new()
        {
            Source = source
        };

        return Task.FromResult(asset);
    }

    public override Task<MaterialAsset> Import(MaterialMetadata metadata)
    {
        string filePath = metadata.FileFullPath;
        using FileStream stream = System.IO.File.OpenRead(filePath);
        Gltf2? gltf = GltfUtils.ParseGltf(stream);

        return Import(gltf!.Materials[0], metadata.FileFullPath);
    }
}
