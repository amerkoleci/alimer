// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using CommunityToolkit.Diagnostics;
using SharpGLTF.Schema2;
using Silk.NET.Assimp;
using GLTF2;
using AssimpScene = Silk.NET.Assimp.Scene;
using GLTFMaterial = SharpGLTF.Schema2.Material;
using GLTFMesh = SharpGLTF.Schema2.Mesh;

namespace Alimer.Assets.Graphics;

/// <summary>
/// Defines a <see cref="MeshAsset"/> importer.
/// </summary>
public sealed class MeshImporter : AssetImporter<MeshAsset, MeshMetadata>
{
    private readonly Assimp _assImp;
    private static readonly PostProcessSteps s_postProcessSteps =
        PostProcessSteps.FindDegenerates
        | PostProcessSteps.FindInvalidData
        //| PostProcessSteps.FlipUVs               // Required for Direct3D
        | PostProcessSteps.FlipWindingOrder
        | PostProcessSteps.JoinIdenticalVertices
        | PostProcessSteps.ImproveCacheLocality
        | PostProcessSteps.OptimizeMeshes
        | PostProcessSteps.Triangulate
        | PostProcessSteps.PreTransformVertices
        | PostProcessSteps.GenerateNormals
        | PostProcessSteps.CalculateTangentSpace
        // | PostProcessSteps.GenerateUVCoords
        // | PostProcessSteps.SortByPrimitiveType
        // | PostProcessSteps.Debone
        ;

    public MeshImporter()
    {
        _assImp = Assimp.GetApi();
    }

    public Task<MeshAsset> ImportGLTF(MeshMetadata metadata)
    {
        using FileStream stream = System.IO.File.OpenRead(metadata.FileFullPath);
        var gltf = GltfUtils.ParseGltf(stream);


        List<Vector3> positions = [];
        List<Vector3> normals = [];
        List<Vector3> tangents = [];
        List<Vector2> texCoords0 = [];
        uint[] indices = [];

        foreach (Gltf2.Mesh mesh in gltf.Meshes)
        {
            foreach (Gltf2.MeshPrimitive primitive in mesh.Primitives)
            {
                if (primitive.Attributes.TryGetValue("POSITION", out int index))
                {
                    var accessor = gltf.Accessors[index];

                    int bufferViewIndex = accessor.BufferView ?? throw new Exception();
                    Gltf2.BufferView bufferView = gltf.BufferViews[bufferViewIndex];

                    int offset = bufferView.ByteOffset + accessor.ByteOffset;

                    //int stride = accessor.ComponentType switch
                    //{
                    //    Gltf2.ComponentType.UnsignedShort => GetCountOfAccessorType(accessor.Type) * sizeof(ushort),
                    //    Gltf2.ComponentType.UnsignedInt => GetCountOfAccessorType(accessor.Type) * sizeof(uint),
                    //    _ => throw new NotSupportedException("This component type is not supported.")
                    //};

                    var buffer = gltf.Buffers[bufferView.Buffer];
                    //return gltf.Buffers[bufferView.Buffer].AsSpan(offset, stride * accessor.Count);
                }


                //bool hasPosition = primitive.GetVertexAccessor("POSITION") is not null;
                //bool hasNormal = primitive.GetVertexAccessor("NORMAL") is not null;
                //bool hasTangent = primitive.GetVertexAccessor("TANGENT") is not null;
                //bool hasTexCoord0 = primitive.GetVertexAccessor("TEXCOORD_0") is not null;

                //Guard.IsTrue(hasPosition);
                //Guard.IsTrue(hasNormal);
                //Guard.IsTrue(hasTexCoord0);

                //IList<Vector3> positionAccessor = primitive.GetVertexAccessor("POSITION").AsVector3Array();
                //IList<Vector3> normalAccessor = primitive.GetVertexAccessor("NORMAL").AsVector3Array();
                //IList<Vector3>? tangentAccessor = hasTangent ? primitive.GetVertexAccessor("TANGENT").AsVector3Array() : default;
                //IList<Vector2> texcoordAccessor = primitive.GetVertexAccessor("TEXCOORD_0").AsVector2Array();
                //var indexAccessor = primitive.GetIndexAccessor().AsIndicesArray();

                //if (!hasTangent)
                //{
                //    Span<Vector3> calculatedTngents = VertexHelper.GenerateTangents(
                //        positionAccessor.ToArray(),
                //        texcoordAccessor.ToArray(),
                //        indexAccessor.ToArray());
                //    tangentAccessor = new List<Vector3>();
                //    for (int i = 0; i < positionAccessor.Count; ++i)
                //    {
                //        tangentAccessor.Add(new Vector3(calculatedTngents[i].X, calculatedTngents[i].Y, calculatedTngents[i].Z));
                //    }
                //}

                //for (int i = 0; i < positionAccessor.Count; ++i)
                //{
                //    Vector3 position = positionAccessor[i];
                //    Vector3 normal = normalAccessor[i];
                //    Vector3 tangent = hasTangent ? tangentAccessor[i]! : Vector3.Zero;
                //    Vector2 texcoord = texcoordAccessor[i];

                //    positions.Add(position);
                //    normals.Add(normal);
                //    tangents.Add(tangent);
                //    texCoords0.Add(texcoord);
                //}

                //// Indices
                //indices = new uint[indexAccessor.Count];

                //for (int i = 0; i < indices.Length; i++)
                //{
                //    indices[i] = indexAccessor[i];
                //}
            }
        }

        ModelRoot modelRoot = ModelRoot.Load(metadata.FileFullPath);

        foreach (GLTFMaterial material in modelRoot.LogicalMaterials)
        {
        }


        foreach (GLTFMesh mesh in modelRoot.LogicalMeshes)
        {
            foreach (MeshPrimitive primitive in mesh.Primitives)
            {
                GLTFMaterial material = primitive.Material;

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

        Span<uint> optimized = stackalloc uint[indices.Length];
        VertexHelper.OptimizeVertexCache(optimized, indices, (uint)positions.Count);

        MeshAsset asset = new()
        {
            Source = metadata.FileFullPath,
            Data = new Rendering.MeshData()
            {
                VertexCount = positions.Count,
                Positions = positions.ToArray(),
                Normals = normals.ToArray(),
                Tangents = tangents.ToArray(),
                Texcoords = texCoords0.ToArray(),
                Indices = optimized.ToArray()
            }
        };

        return Task.FromResult(asset);
    }

    public unsafe Task<MeshAsset> ImportAssimp(string source, IServiceRegistry services)
    {
        //AssimpScene* scene = _assImp.ImportFileFromMemory(data.AsSpan(), (uint)data.Length, (uint)s_postProcessSteps, (byte*)null);
        AssimpScene* scene = _assImp.ImportFile(source, (uint)s_postProcessSteps);

        if (scene is null || scene->MFlags == (uint)SceneFlags.Incomplete || scene->MRootNode is null)
        {
            throw new InvalidDataException(_assImp.GetErrorStringS());
        }

        Silk.NET.Assimp.Mesh* mesh = scene->MMeshes[0];
        Guard.IsTrue(mesh->MVertices is not null);
        Guard.IsTrue(mesh->MNormals is not null);

        List<Vector3> positions = [];
        List<Vector3> normals = [];
        List<Vector3> tangents = [];
        List<Vector2> texCoords0 = [];
        uint[] indices = [];

        bool hasTangentsAndBitangents = mesh->MTangents is not null;
        bool hasHasTexCoords0 = mesh->MTextureCoords[0] is not null;

        for (int i = 0; i < (int)mesh->MNumVertices; ++i)
        {
            Vector3 position = mesh->MVertices[i];
            Vector3 normal = mesh->MNormals[i];
            Vector3 tangent = Vector3.Zero;
            Vector2 texcoord = Vector2.Zero;

            if (hasTangentsAndBitangents)
            {
                tangent = mesh->MTangents[i];
            }

            if (hasHasTexCoords0)
            {
                texcoord = new Vector2(mesh->MTextureCoords[0][i].X, 1.0f - mesh->MTextureCoords[0][i].Y);
            }

            positions.Add(position);
            normals.Add(normal);
            tangents.Add(tangent);
            texCoords0.Add(texcoord);
        }

        indices = new uint[(int)mesh->MNumFaces];
        for (int i = 0; i < (int)mesh->MNumFaces; ++i)
        {
            Guard.IsTrue(mesh->MFaces[i].MNumIndices == 3);
            indices[i + 0] = mesh->MFaces[i].MIndices[0];
            indices[i + 1] = mesh->MFaces[i].MIndices[1];
            indices[i + 2] = mesh->MFaces[i].MIndices[2];
        }

        MeshAsset asset = new()
        {
            Source = source,
            Data = new Rendering.MeshData()
            {
                VertexCount = positions.Count,
                Positions = [.. positions],
                Normals = [.. normals],
                Tangents = [.. tangents],
                Texcoords = [.. texCoords0],
                Indices = indices
            }
        };

        return Task.FromResult(asset);
    }

    public override Task<MeshAsset> Import(MeshMetadata metadata)
    {
        return ImportGLTF(metadata);
    }
}
