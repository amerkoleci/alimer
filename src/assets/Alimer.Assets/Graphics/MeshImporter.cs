// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using CommunityToolkit.Diagnostics;
using Silk.NET.Assimp;
using GLTF2;
using Texture = Alimer.Graphics.Texture;
using Mesh = Alimer.Rendering.Mesh;
using Material = Alimer.Rendering.Material;
using AssimpScene = Silk.NET.Assimp.Scene;
using AssimpMaterial = Silk.NET.Assimp.Material;
using Alimer.Rendering;
using MeshOptimizer;
using Alimer.Utilities;
using System.Runtime.InteropServices;
using SkiaSharp;
using Alimer.Graphics;

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

    public MeshImporter(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        _assImp = Assimp.GetApi();
    }

    public GraphicsDevice Device { get; }

    public Task<MeshAsset> ImportGLTF(MeshMetadata metadata)
    {
        string filePath = metadata.FileFullPath;
        using FileStream stream = System.IO.File.OpenRead(filePath);
        using FileStream binaryDataStream = System.IO.File.OpenRead(filePath);
        Gltf2? gltf = GltfUtils.ParseGltf(stream);

        VertexPositionNormalTangentTexture[] vertices = [];
        Vector2[] vertexTexCoords1 = [];
        uint[] indices = [];

        byte[][] buffers = new byte[gltf!.Buffers.Length][];
        for (int i = 0; i < gltf.Buffers.Length; i++)
        {
            buffers[i] = GltfUtils.LoadBinaryBuffer(binaryDataStream);
        }

        for (int meshIndex = 0; meshIndex < gltf.Meshes.Length; meshIndex++)
        {
            Gltf2.Mesh mesh = gltf.Meshes[meshIndex];
            foreach (Gltf2.MeshPrimitive primitive in mesh.Primitives)
            {
                int vertexOffset = vertices.Length;

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
                        Array.Resize(ref vertices, vertexOffset + vertexCount);
                        Span<Vector3> vector3Data = MemoryMarshal.Cast<byte, Vector3>(attributeData);
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            vertices[vertexOffset + i].Position = vector3Data[i];
                        }
                    }
                    else if (attributeName == "NORMAL")
                    {
                        Array.Resize(ref vertices, vertexOffset + vertexCount);

                        Span<Vector3> vector3Data = MemoryMarshal.Cast<byte, Vector3>(attributeData);
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            vertices[vertexOffset + i].Normal = vector3Data[i];
                        }
                    }
                    else if (attributeName == "TANGENT")
                    {
                        Array.Resize(ref vertices, vertexOffset + vertexCount);

                        Span<Vector4> vector4Data = MemoryMarshal.Cast<byte, Vector4>(attributeData);
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            vertices[vertexOffset + i].Tangent = new Vector3(vector4Data[i].X, vector4Data[i].Y, vector4Data[i].Z);
                        }
                    }
                    else if (attributeName == "TEXCOORD_0")
                    {
                        // TODO: Handle TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE and TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                        Array.Resize(ref vertices, vertexOffset + vertexCount);

                        if (accessor.ComponentType == Gltf2.ComponentType.Float)
                        {
                            Span<Vector2> vector2Data = MemoryMarshal.Cast<byte, Vector2>(attributeData);
                            for (int i = 0; i < vertexCount; ++i)
                            {
                                vertices[vertexOffset + i].TextureCoordinate = vector2Data[i];
                            }
                        }
                        else if (accessor.ComponentType == Gltf2.ComponentType.UnsignedByte)
                        {
                            for (int i = 0; i < vertexCount; ++i)
                            {
                                byte s = attributeData[i + 0];
                                byte t = attributeData[i + 1];

                                vertices[vertexOffset + i].TextureCoordinate = new Vector2(s / 255.0f, t / 255.0f);
                            }
                        }
                        else if (accessor.ComponentType == Gltf2.ComponentType.UnsignedShort)
                        {
                            Span<ushort> ushortData = MemoryMarshal.Cast<byte, ushort>(attributeData);
                            for (int i = 0; i < vertexCount; ++i)
                            {
                                ushort s = ushortData[i + 0];
                                ushort t = ushortData[i + 1];

                                vertices[vertexOffset + i].TextureCoordinate = new Vector2(s / 65535.0f, t / 65535.0f);
                            }
                        }
                    }
                    else if (attributeName == "TEXCOORD_1")
                    {
                        // TODO: Handle TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE and TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                        Array.Resize(ref vertexTexCoords1, vertexOffset + vertexCount);

                        Span<Vector2> vector2Data = MemoryMarshal.Cast<byte, Vector2>(attributeData);
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            vertexTexCoords1[vertexOffset + i] = vector2Data[i];
                        }
                    }
                    else if (attributeName == "JOINTS_0")
                    {
                    }
                    else if (attributeName == "WEIGHTS_0")
                    {
                    }
                    else if (attributeName == "COLOR_0")
                    {
                    }
                }

                if (!hasTangent)
                {
                    Vector3[] vertexPositions = new Vector3[vertices.Length];
                    Vector2[] vertexTexCoords0 = new Vector2[vertices.Length];
                    for(int i = 0; i < vertices.Length; ++i)
                    {
                        vertexPositions[i] = vertices[i].Position;
                        vertexTexCoords0[i] = vertices[i].TextureCoordinate;
                    }

                    Vector3[] vertexTangents = new Vector3[vertices.Length];
                    VertexHelper.GenerateTangents(
                        vertexTangents,
                        vertexPositions,
                        vertexTexCoords0,
                        indices);

                    for (int i = 0; i < vertices.Length; ++i)
                    {
                        vertices[i].Tangent = vertexTangents[i];
                    }
                }
            }

            //Gltf2.Node node = gltf.Nodes.First(n => n.Mesh == meshIndex);
            //float[] matrix = node.Matrix;
        }

        Material[] materials = new Material[gltf.Materials.Length];
        for (int materialIndex = 0; materialIndex < gltf.Materials.Length; materialIndex++)
        {
            Gltf2.Material material = gltf.Materials[materialIndex];
            if (material.PbrMetallicRoughness.HasValue)
            {
                Gltf2.MaterialPbrMetallicRoughness pbr = material.PbrMetallicRoughness.Value;
                PhysicallyBasedMaterial physicallyBasedMaterial = new()
                {
                    Name = material.Name ?? string.Empty,
                    BaseColorFactor = pbr.BaseColorFactor,
                    MetallicFactor = pbr.MetallicFactor,
                    RoughnessFactor = pbr.RoughnessFactor,
                    EmissiveFactor = material.EmissiveFactor,
                    AlphaMode = FromGltf(material.AlphaMode),
                    AlphaCutoff = material.AlphaCutoff,
                    DoubleSided = material.DoubleSided
                };

                if (pbr.BaseColorTexture.HasValue)
                {
                    Gltf2.TextureInfo baseColorTexture = pbr.BaseColorTexture.Value;
                    //Texture diffuseTexture = await GetTextureAsync(diffuseTextureIndex.Value);
                    physicallyBasedMaterial.BaseColorTexture = GetTexture(gltf, buffers, baseColorTexture.Index);
                    //baseColorTexture.TexCoord;
                }

                if (pbr.MetallicRoughnessTexture.HasValue)
                {
                    Gltf2.TextureInfo metallicRoughnessTexture = pbr.MetallicRoughnessTexture.Value;
                    physicallyBasedMaterial.MetallicRoughnessTexture = GetTexture(gltf, buffers, metallicRoughnessTexture.Index);
                }

                if (material.NormalTexture.HasValue)
                {
                    Gltf2.MaterialNormalTextureInfo normalTextureInfo = material.NormalTexture.Value;
                    physicallyBasedMaterial.NormalTexture = GetTexture(gltf, buffers, normalTextureInfo.Index);
                    physicallyBasedMaterial.NormalScale = normalTextureInfo.Scale;
                }

                if (material.OcclusionTexture.HasValue)
                {
                    Gltf2.MaterialOcclusionTextureInfo occlusionTextureInfo = material.OcclusionTexture.Value;
                    physicallyBasedMaterial.OcclusionTexture = GetTexture(gltf, buffers, occlusionTextureInfo.Index);
                    physicallyBasedMaterial.OcclusionStrength = occlusionTextureInfo.Strength;
                }

                if (material.EmissiveTexture.HasValue)
                {
                    Gltf2.TextureInfo emissiveTextureInfo = material.EmissiveTexture.Value;
                    physicallyBasedMaterial.EmissiveTexture = GetTexture(gltf, buffers, emissiveTextureInfo.Index);
                }

                materials[materialIndex] = physicallyBasedMaterial;
            }
        }

        // Create skins
        foreach (Gltf2.Skin skin in gltf.Skins)
        {
        }

        // Create animations
        foreach (Gltf2.Animation animation in gltf.Animations)
        {
        }

        // Cameras
        for (int camerasIndex = 0; camerasIndex < gltf.Cameras.Length; camerasIndex++)
        {
            Gltf2.Camera camera = gltf.Cameras[camerasIndex];
        }

        Span<uint> optimized = stackalloc uint[indices.Length];
        OptimizeVertexCache(optimized, indices, (uint)vertices.Length);

        Mesh engineMesh = new(vertices.Length, VertexPositionNormalTangentTexture.VertexAttributes, indices.Length, IndexFormat.Uint32);
        engineMesh.SetVertices(vertices);
        engineMesh.SetIndices(optimized);
        engineMesh.RecalculateBounds();
        engineMesh.CreateGpuData(Device);

        MeshAsset asset = new(engineMesh, materials)
        {
            Source = metadata.FileFullPath,
            Translation = gltf.Nodes[0].Translation,
            Rotation = gltf.Nodes[0].Rotation,
            Scale = gltf.Nodes[0].Scale
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

        throw new NotImplementedException();
        //MeshData meshData = new()
        //{
        //    VertexCount = positions.Count,
        //    Positions = [.. positions],
        //    Normals = [.. normals],
        //    Tangents = [.. tangents],
        //    Texcoords = [.. texCoords0],
        //    Indices = optimized.ToArray()
        //};

        //MeshAsset asset = new(meshData, materials)
        //{
        //    Source = source
        //};

        //return Task.FromResult(asset);
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

        if (accessor.Sparse.HasValue)
        {
            Gltf2.AccessorSparse sparse = accessor.Sparse.Value;
            Gltf2.BufferView sparse_indices_view = gltf.BufferViews[sparse.Indices.BufferView];
            Gltf2.BufferView sparse_values_view = gltf.BufferViews[sparse.Values.BufferView];
            Gltf2.Buffer sparse_indices_buffer = gltf.Buffers[sparse_indices_view.Buffer];
            Gltf2.Buffer sparse_values_buffer = gltf.Buffers[sparse_values_view.Buffer];
            //const uint8_t* sparse_indices_data = sparse_indices_buffer.data.data() + sparse.indices.byteOffset + sparse_indices_view.byteOffset;
            //const uint8_t* sparse_values_data = sparse_values_buffer.data.data() + sparse.values.byteOffset + sparse_values_view.byteOffset;
            //switch (sparse.indices.componentType)
            //{
            //    default:
            //    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            //        for (int s = 0; s < sparse.count; ++s)
            //        {
            //            mesh.vertex_positions[sparse_indices_data[s]] = ((const XMFLOAT3*)sparse_values_data)[s] ;
            //}
            //break;
            //
            //            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            //    for (int s = 0; s < sparse.count; ++s)
            //    {
            //        mesh.vertex_positions[((const uint16_t*)sparse_indices_data)[s]] = ((const XMFLOAT3*)sparse_values_data)[s] ;
            //}
            //break;
            //
            //            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            //    for (int s = 0; s < sparse.count; ++s)
            //    {
            //        mesh.vertex_positions[((const uint32_t*)sparse_indices_data)[s]] = ((const XMFLOAT3*)sparse_values_data)[s] ;
            //}
            //break;
            //}
            throw new NotImplementedException("Sparse accessors are not yet supported.");
        }

        return buffers[bufferView.Buffer].AsSpan(offset, stride * accessor.Count);
    }

    // TODO: async
    private Texture GetTexture(Gltf2 gltf, byte[][] buffers, int textureIndex)
    {
        int imageIndex = gltf.Textures[textureIndex].Source ?? throw new Exception();
        Gltf2.Image image = gltf.Images[imageIndex];

        int bufferViewIndex = image.BufferView ?? throw new Exception();
        Gltf2.BufferView bufferView = gltf.BufferViews[bufferViewIndex];

        byte[] currentBuffer = buffers[bufferView.Buffer];

        Span<byte> imageData = currentBuffer.AsSpan(bufferView.ByteOffset, bufferView.ByteLength);
        Texture texture = Texture.FromMemory(Device, imageData);
        return texture;
    }

    private static MaterialAlphaMode FromGltf(Gltf2.AlphaMode alphaMode)
    {
        return alphaMode switch
        {
            Gltf2.AlphaMode.Opaque => MaterialAlphaMode.Opaque,
            Gltf2.AlphaMode.Mask => MaterialAlphaMode.Mask,
            Gltf2.AlphaMode.Blend => MaterialAlphaMode.Blend,
            _ => throw new NotSupportedException($"Unsupported GLTF alpha mode: {alphaMode}")
        };
    }

    #endregion
}
