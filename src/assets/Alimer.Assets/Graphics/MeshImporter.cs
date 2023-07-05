// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SharpGLTF.Schema2;
using SkiaSharp;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MeshAsset"/> importer.
/// </summary>
public sealed class MeshImporter : AssetImporter<MeshAsset>
{
    public override Task<MeshAsset> Import(string source, IServiceProvider services)
    {
        ModelRoot modelRoot = SharpGLTF.Schema2.ModelRoot.Load(source);

        MeshAsset asset = new();

        return Task.FromResult(asset);
    }
}
