// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Assets.Graphics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Physics;
using Alimer.Rendering;

namespace Alimer.Samples;

[Description("Engine - Scene PBR Renderer")]
public sealed class ScenePBRRendererSample : SampleBase
{
    private readonly Entity _cameraEntity;
    private readonly Entity _damagedHelmetEntity;

    public ScenePBRRendererSample(IServiceRegistry services)
        : base("Engine - Scene Cube")
    {
        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        Entity root = new();

        // Camera
        _cameraEntity = new();
        CameraComponent camera = _cameraEntity.AddComponent<CameraComponent>();
        camera.Entity!.Transform.Position = new Vector3(0.0f, 0.0f, 5.0f);
        root.Children.Add(_cameraEntity);

        // GLTF mesh
        string meshesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Meshes");

        MeshImporter meshImporter = new(GraphicsDevice);
        MeshMetadata meshMetadata = new()
        {
            FileFullPath = Path.Combine(meshesPath, "DamagedHelmet.glb")
        };
        MeshAsset meshAsset = meshImporter.Import(meshMetadata).Result;

        Span<VertexPositionNormalTangentTexture> vertices = stackalloc VertexPositionNormalTangentTexture[meshAsset.Data!.VertexCount];
        for (int i = 0; i < meshAsset.Data.VertexCount; i++)
        {
            vertices[i] = new VertexPositionNormalTangentTexture(
                meshAsset.Data.Positions[i],
                meshAsset.Data.Normals[i],
                meshAsset.Data.Tangents[i],
                meshAsset.Data.Texcoords[i]
                );
        }

        Mesh damagedHelmetMesh = new(vertices.Length, VertexPositionNormalTangentTexture.VertexAttributes, meshAsset.Data.Indices!.Length, IndexFormat.Uint32);
        damagedHelmetMesh.SetVertices(vertices);
        damagedHelmetMesh.SetIndices(meshAsset.Data.Indices!.AsSpan());
        damagedHelmetMesh.RecalculateBounds();
        damagedHelmetMesh.CreateGpuData(GraphicsDevice);
        _= ToDispose(damagedHelmetMesh);

        {
            _damagedHelmetEntity = new("Damaged Helmet", new Vector3(0.0f, 2.0f, 0.0f));

            MeshComponent meshComponent = new(damagedHelmetMesh);

            foreach(var material in meshAsset.Materials)
            {
                meshComponent.Materials.Add(ToDispose(material));
            }

            _damagedHelmetEntity.AddComponent(meshComponent);
            root.Children.Add(_damagedHelmetEntity);
        }

        Scene.RootEntity = root;
    }

    public IServiceRegistry Services { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }

    public override void Update(GameTime time)
    {
        float deltaTime = (float)time.Elapsed.TotalSeconds;
        //_damagedHelmetEntity.Transform.Rotate(10 * deltaTime, 20 * deltaTime, 30 * deltaTime);
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        //_cubeEntity.GetComponent<RigidBodyComponent>().LinearVelocity = new Vector3(100, -100.0f, 0.0f);
    }
}
