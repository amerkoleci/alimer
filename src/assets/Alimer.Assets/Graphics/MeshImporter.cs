// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using CommunityToolkit.Diagnostics;
using SharpGLTF.Schema2;
using Silk.NET.Assimp;
using GLTF2;
using Material = Alimer.Rendering.Material;
using AssimpScene = Silk.NET.Assimp.Scene;
using AssimpMaterial = Silk.NET.Assimp.Material;
using GLTFMaterial = SharpGLTF.Schema2.Material;
using GLTFMesh = SharpGLTF.Schema2.Mesh;
using Alimer.Rendering;
using MeshOptimizer;
using Alimer.Utilities;
using Alimer.Numerics;
using System.Runtime.InteropServices;

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
        string filePath = metadata.FileFullPath;
        using FileStream stream = System.IO.File.OpenRead(filePath);
        using FileStream binaryDataStream = System.IO.File.OpenRead(filePath);
        Gltf2? gltf = GltfUtils.ParseGltf(stream);

        Vector3[] positions = [];
        List<Vector3> normals = [];
        List<Vector3> tangents = [];
        List<Vector2> texCoords0 = [];
        uint[] indices = [];

        byte[][] buffers = new byte[gltf!.Buffers.Length][];
        for (int i = 0; i < gltf.Buffers.Length; i++)
        {
            buffers[i] = GltfUtils.LoadBinaryBuffer(binaryDataStream);
        }

        foreach (Gltf2.Mesh mesh in gltf.Meshes)
        {
            foreach (Gltf2.MeshPrimitive primitive in mesh.Primitives)
            {
                int vertexOffset = positions.Length;

                bool hasPosition = primitive.Attributes.TryGetValue("POSITION", out int positionIndex);
                bool hasNormal = primitive.Attributes.TryGetValue("NORMAL", out int normalIndex);
                bool hasTangent = primitive.Attributes.TryGetValue("TANGENT", out int tangentIndex);
                bool hasTexCoord0 = primitive.Attributes.TryGetValue("TEXCOORD_0", out int texCoord0Index);

                Guard.IsTrue(hasPosition);
                Guard.IsTrue(hasNormal);
                Guard.IsTrue(hasTexCoord0);

                if (primitive.Indices.HasValue)
                {
                    int indicesIndex = primitive.Indices.Value;
                    Gltf2.Accessor accessor = gltf.Accessors[indicesIndex];
                    int indexCount = accessor.Count;

                    int indexOffset = indices.Length;
                    Array.Resize(ref indices, indexOffset + indexCount);
                    //mesh.subsets.back().indexOffset = (uint32_t)indexOffset;
                    //mesh.subsets.back().indexCount = (uint32_t)indexCount;

                    Span<byte> data = GetAccessorData(gltf, buffers, accessor, out int stride);
                    if (stride == 1)
                    {
                        for (int i = 0; i < indexCount; i += 3)
                        {
                            indices[indexOffset + i + 0] = (uint)vertexOffset + data[i + 0];
                            indices[indexOffset + i + 1] = (uint)vertexOffset + data[i + 1];
                            indices[indexOffset + i + 2] = (uint)vertexOffset + data[i + 2];
                        }
                    }
                    else if (stride == 2)
                    {
                        Span<ushort> ushortData = MemoryMarshal.Cast<byte, ushort>(data);
                        for (int i = 0; i < indexCount; i += 3)
                        {
                            indices[indexOffset + i + 0] = (uint)vertexOffset + ushortData[i + 0];
                            indices[indexOffset + i + 1] = (uint)vertexOffset + ushortData[i + 1];
                            indices[indexOffset + i + 2] = (uint)vertexOffset + ushortData[i + 2];
                        }
                    }
                    else if (stride == 4)
                    {
                        Span<uint> uintData = MemoryMarshal.Cast<byte, uint>(data);
                        for (int i = 0; i < indexCount; i += 3)
                        {
                            indices[indexOffset + i + 0] = (uint)vertexOffset + uintData[i + 0];
                            indices[indexOffset + i + 1] = (uint)vertexOffset + uintData[i + 1];
                            indices[indexOffset + i + 2] = (uint)vertexOffset + uintData[i + 2];
                        }
                    }
                }

                // Load vertex attributes now
                foreach (var attribute in primitive.Attributes)
                {
                    string attributeName = attribute.Key;
                    int accessorIndex = attribute.Value;
                    Gltf2.Accessor accessor = gltf.Accessors[accessorIndex];
                    int vertexCount = accessor.Count;

                    Span<byte> attributeData = GetAccessorData(gltf, buffers, accessor, out int stride);
                    // Process the data based on the attribute name (e.g., "POSITION", "NORMAL", etc.)
                    // and the accessor type (e.g., VEC3, VEC2, etc.) to populate the positions, normals, tangents, and texCoords0 lists.

                    if (attributeName == "POSITION")
                    {
                        Array.Resize(ref positions, vertexOffset + vertexCount);
                        Span<Vector3> vector3Data = MemoryMarshal.Cast<byte, Vector3>(attributeData);
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            positions[vertexOffset + i] = vector3Data[i];
                        }
                    }
                }
            }
        }

        Material[] materials = new Material[gltf.Materials.Length];

        ModelRoot modelRoot = ModelRoot.Load(metadata.FileFullPath);


        foreach (GLTFMesh mesh in modelRoot.LogicalMeshes)
        {
            foreach (MeshPrimitive primitive in mesh.Primitives)
            {
                GLTFMaterial material = primitive.Material;

                bool hasNormal = primitive.GetVertexAccessor("NORMAL") is not null;
                bool hasTangent = primitive.GetVertexAccessor("TANGENT") is not null;
                bool hasTexCoord0 = primitive.GetVertexAccessor("TEXCOORD_0") is not null;

                Guard.IsTrue(hasNormal);
                Guard.IsTrue(hasTexCoord0);

                IList<Vector3> positionAccessor = primitive.GetVertexAccessor("POSITION").AsVector3Array();
                IList<Vector3> normalAccessor = primitive.GetVertexAccessor("NORMAL").AsVector3Array();
                IList<Vector3>? tangentAccessor = hasTangent ? primitive.GetVertexAccessor("TANGENT").AsVector3Array() : default;
                IList<Vector2> texcoordAccessor = primitive.GetVertexAccessor("TEXCOORD_0").AsVector2Array();

                if (!hasTangent)
                {
                    Span<Vector3> calculatedTangents = new Vector3[positionAccessor.Count];
                    VertexHelper.GenerateTangents(
                        calculatedTangents,
                        positionAccessor,
                        texcoordAccessor,
                        indices);
                    tangentAccessor = new List<Vector3>();
                    for (int i = 0; i < positionAccessor.Count; ++i)
                    {
                        tangentAccessor.Add(calculatedTangents[i]);
                    }
                }

                for (int i = 0; i < positionAccessor.Count; ++i)
                {
                    Vector3 normal = normalAccessor[i];
                    Vector3 tangent = hasTangent ? tangentAccessor[i]! : Vector3.Zero;
                    Vector2 texcoord = texcoordAccessor[i];

                    normals.Add(normal);
                    tangents.Add(tangent);
                    texCoords0.Add(texcoord);
                }
            }
        }

        Span<uint> optimized = stackalloc uint[indices.Length];
        OptimizeVertexCache(optimized, indices, (uint)positions.Length);

        MeshData meshData = new()
        {
            VertexCount = positions.Length,
            Positions = positions,
            Normals = normals.ToArray(),
            Tangents = tangents.ToArray(),
            Texcoords = texCoords0.ToArray(),
            Indices = optimized.ToArray()
        };

        MeshAsset asset = new(meshData, materials)
        {
            Source = metadata.FileFullPath
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

        // Mesh optimizations
        Span<uint> optimized = stackalloc uint[indices.Length];
        OptimizeVertexCache(optimized, indices, (uint)positions.Count);

        Material[] materials = new Material[scene->MNumMaterials];

        MeshData meshData = new()
        {
            VertexCount = positions.Count,
            Positions = [.. positions],
            Normals = [.. normals],
            Tangents = [.. tangents],
            Texcoords = [.. texCoords0],
            Indices = optimized.ToArray()
        };

        MeshAsset asset = new(meshData, materials)
        {
            Source = source
        };

        return Task.FromResult(asset);
    }

    public override Task<MeshAsset> Import(MeshMetadata metadata)
    {
        return ImportGLTF(metadata);
    }

    private static void OptimizeVertexCache(Span<uint> destination, ReadOnlySpan<uint> indices, uint vertexCount)
    {
        // https://github.com/zeux/meshoptimizer#vertex-cache-optimization
        Meshopt.OptimizeVertexCache(destination, indices, vertexCount);
    }

    #region GLTF
    private static Span<byte> GetAccessorData(Gltf2 gltf, byte[][] buffers, Gltf2.Accessor accessor, out int stride)
    {
        int bufferViewIndex = accessor.BufferView ?? throw new Exception();
        Gltf2.BufferView bufferView = gltf.BufferViews[bufferViewIndex];

        int offset = bufferView.ByteOffset + accessor.ByteOffset;

        stride = accessor.ByteStride(bufferView);

        return buffers[bufferView.Buffer].AsSpan(offset, stride * accessor.Count);
    }
    #endregion
}
