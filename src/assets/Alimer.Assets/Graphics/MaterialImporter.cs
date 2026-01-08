// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SharpGLTF.Schema2;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MaterialAsset"/> importer.
/// </summary>
public sealed class MaterialImporter : AssetImporter<MaterialAsset, MaterialMetadata>
{
    public Task<MaterialAsset> Import(SharpGLTF.Schema2.Material gltfMaterial, string source)
    {
        MaterialAsset asset = new()
        {
            Source = source
        };

        return Task.FromResult(asset);
    }

    public override Task<MaterialAsset> Import(MaterialMetadata metadata)
    {
        //GraphicsDevice device = services.GetRequiredService<GraphicsDevice>();
        ModelRoot modelRoot = ModelRoot.Load(metadata.FileFullPath);

        return Import(modelRoot.LogicalMaterials[0], metadata.FileFullPath);
    }
}
