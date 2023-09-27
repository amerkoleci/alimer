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
    public override Task<MeshAsset> Import(string source, IServiceRegistry services)
    {
        GraphicsDevice device = services.GetService<GraphicsDevice>();
        ModelRoot modelRoot = ModelRoot.Load(source);

        foreach (Material material in modelRoot.LogicalMaterials)
        {
        }

        List<Vector3> positions = [];
        List<Vector3> normals = [];
        List<Vector3> tangents = [];
        List<Vector2> texCoords0 = [];
        uint[] indices = [];

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
                Guard.IsTrue(hasNormal);
                Guard.IsTrue(hasTexCoord0);

                IList<Vector3> positionAccessor = primitive.GetVertexAccessor("POSITION").AsVector3Array();
                IList<Vector3> normalAccessor = primitive.GetVertexAccessor("NORMAL").AsVector3Array();
                IList<Vector3>? tangentAccessor = hasTangent ? primitive.GetVertexAccessor("TANGENT").AsVector3Array() : default;
                IList<Vector2> texcoordAccessor = primitive.GetVertexAccessor("TEXCOORD_0").AsVector2Array();
                var indexAccessor = primitive.GetIndexAccessor().AsIndicesArray();

                if (!hasTangent)
                {
                    Span<Vector3> calculatedTngents = VertexHelper.GenerateTangents(
                        positionAccessor.ToArray(),
                        texcoordAccessor.ToArray(),
                        indexAccessor.ToArray());
                    tangentAccessor = new List<Vector3>();
                    for (int i = 0; i < positionAccessor.Count; ++i)
                    {
                        tangentAccessor.Add(new Vector3(calculatedTngents[i].X, calculatedTngents[i].Y, calculatedTngents[i].Z));
                    }
                }

                for (int i = 0; i < positionAccessor.Count; ++i)
                {
                    Vector3 position = positionAccessor[i];
                    Vector3 normal = normalAccessor[i];
                    Vector3 tangent = hasTangent ? tangentAccessor[i]! : Vector3.Zero;
                    Vector2 texcoord = texcoordAccessor[i];

                    positions.Add(position);
                    normals.Add(normal);
                    tangents.Add(tangent);
                    texCoords0.Add(texcoord);
                }

                // Indices
                indices = new uint[indexAccessor.Count];

                for (int i = 0; i < indices.Length; i++)
                {
                    indices[i] = indexAccessor[i];
                }
            }
        }

        MeshAsset asset = new()
        {
            Source = source,
            VertexCount = positions.Count,
            Positions = positions.ToArray(),
            Normals = normals.ToArray(),
            Tangents = tangents.ToArray(),
            Texcoords = texCoords0.ToArray(),
            Indices = indices
        };

        return Task.FromResult(asset);
    }
}
