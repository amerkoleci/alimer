// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Physics;
using Alimer.Rendering;

namespace Alimer.Samples;

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
        root.Children.Add(_cameraEntity);

        //var test = _cameraEntity.Serialize();

        // Floor
        RigidBodyComponent floorRigidBody = new()
        {
            ColliderShape = new BoxColliderShape(new Vector3(200.0f, 2.0f, 200.0f)),
            MotionType = MotionType.Static
        };

        var floorEntity = new Entity("Floor", new Vector3(0.0f, -1.0f, 0.0f));
        floorEntity.Add(floorRigidBody);
        root.Children.Add(floorEntity);

        // Mesh
        RigidBodyComponent sphereRigidBody = new()
        {
            ColliderShape = new SphereColliderShape(50.0f),
            //Mass = 100.0f
        };

        _meshEntity = new("Sphere", new Vector3(0.0f, 2.0f, 0.0f));

        MeshComponent meshComponent = new()
        {
            Mesh = new Mesh(GraphicsDevice)
        };

        _meshEntity.Add(meshComponent).Add(sphereRigidBody);
        
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
