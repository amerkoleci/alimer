// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using SharpGLTF.Schema2;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MeshAsset"/> importer.
/// </summary>
public sealed class MeshImporter : AssetImporter<MeshAsset>
{
    public override Task<MeshAsset> Import(string source, IServiceProvider services)
    {
        //GraphicsDevice device = services.GetRequiredService<GraphicsDevice>();
        ModelRoot modelRoot = ModelRoot.Load(source);

        MeshAsset asset = new()
        {
            Source = source
        };


        foreach (Mesh mesh in modelRoot.LogicalMeshes)
        {
            foreach (MeshPrimitive primitive in mesh.Primitives)
            {
                Material material = primitive.Material;

                bool hasPosition = primitive.GetVertexAccessor("POSITION") is not null;
                bool hasNormal = primitive.GetVertexAccessor("NORMAL") is not null;
                bool hasTangent = primitive.GetVertexAccessor("TANGENT") is not null;
                bool hasTexCoord0 = primitive.GetVertexAccessor("TEXCOORD_0") is not null;

                Guard.IsTrue(hasPosition);

                IList<Vector3> positionAccessor = primitive.GetVertexAccessor("POSITION").AsVector3Array();
            }
        }

        return Task.FromResult(asset);
    }
}
