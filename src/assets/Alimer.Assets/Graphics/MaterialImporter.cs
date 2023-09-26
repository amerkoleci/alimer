// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using CommunityToolkit.Diagnostics;
using SharpGLTF.Schema2;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MaterialAsset"/> importer.
/// </summary>
public sealed class MaterialImporter : AssetImporter<MaterialAsset>
{
    public Task<MaterialAsset> Import(SharpGLTF.Schema2.Material gltfMaterial, string source)
    {
        MaterialAsset asset = new()
        {
            Source = source
        };

        return Task.FromResult(asset);
    }

    public override Task<MaterialAsset> Import(string source, IServiceProvider services)
    {
        //GraphicsDevice device = services.GetRequiredService<GraphicsDevice>();
        ModelRoot modelRoot = ModelRoot.Load(source);

        return Import(modelRoot.LogicalMaterials[0], source);
    }
}
