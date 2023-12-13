// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Physics;
using Alimer.Rendering;

namespace Alimer.Samples.Engime;

[Description("Engine - Scene Cube")]
public sealed class SceneCubeSample : SampleBase
{
    private readonly Entity _cameraEntity;
    private readonly Entity _meshEntity;

    public SceneCubeSample(IServiceRegistry services)
        : base("Engine - Scene Cube")
    {
        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        Entity root = new();

        // Camera
        _cameraEntity = new();
        _ = _cameraEntity.Add<CameraComponent>();

        //var test = _cameraEntity.Serialize();

        // Mesh
        _meshEntity = new();

        MeshComponent meshComponent = new()
        {
            Mesh = new Mesh(GraphicsDevice)
        };

        RigidBodyComponent rigidBody = new()
        {
            //ColliderShape = new SphereColliderShape(50.0f),
            //Mass = 100.0f
        };

        _meshEntity.Add(meshComponent).Add(rigidBody);

        root.Children.Add(_cameraEntity);
        root.Children.Add(_meshEntity);

        Scene.RootEntity = root;
    }

    public IServiceRegistry Services { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }

    public override void Draw(RenderContext context, Texture swapChainTexture)
    {
    }
}
