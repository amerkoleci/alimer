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
    private readonly Entity _cubeEntity;

    // Create grid
    int xCount = 4;
    int yCount = 4;
    const float step = 4.0f;
    private readonly List<Entity> _gridEntities = [];

    public SceneCubeSample(IServiceRegistry services)
        : base("Engine - Scene Cube")
    {
        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        Entity root = new();

        // Camera
        _cameraEntity = new();
        CameraComponent camera = _cameraEntity.AddComponent<CameraComponent>();
        camera.Entity!.Transform.Position = new Vector3(0.0f, 0.0f, 25.0f);
        root.Children.Add(_cameraEntity);

        //var test = _cameraEntity.Serialize();

        // Floor
        RigidBodyComponent floorRigidBody = new()
        {
            ColliderShape = new BoxColliderShape(new Vector3(100.0f, 1.0f, 100.0f)),
            BodyType = RigidBodyType.Static
        };

        var floorEntity = new Entity("Floor", new Vector3(0.0f, -1.0f, 0.0f));
        floorEntity.AddComponent(floorRigidBody);
        root.Children.Add(floorEntity);

        // Cube mesh
        Mesh cubeMesh = ToDispose(Mesh.CreateCube(5.0f));
        cubeMesh.CreateGpuData(GraphicsDevice);

        PhysicallyBasedMaterial sharedMaterial = new()
        {
            BaseColorFactor = Colors.White,
        };

        {
            _cubeEntity = new("Cube", new Vector3(0.0f, 2.0f, 0.0f));

            RigidBodyComponent cubeRigidBody = new()
            {
                ColliderShape = new SphereColliderShape(5.0f),
                //Mass = 100.0f
            };

            MeshComponent meshComponent = new(cubeMesh);
            meshComponent.Materials.Add(sharedMaterial);
            _cubeEntity.AddComponent(meshComponent);
            //_cubeEntity.AddComponent(cubeRigidBody);
            root.Children.Add(_cubeEntity);
        }

        // Cube 2 mesh
        {
            Mesh smallCubeMesh = ToDispose(Mesh.CreateCube(1.0f));
            smallCubeMesh.CreateGpuData(GraphicsDevice);

            for (int x = 0; x < xCount; x++)
            {
                for (int y = 0; y < yCount; y++)
                {
                    Vector3 translation = new(step * (x - xCount / 2 + 0.5f), step * (y - yCount / 2 + 0.5f), 0);

                    Entity entity = new("Cube", translation);
                    MeshComponent meshComponent = new(smallCubeMesh);
                    meshComponent.Materials.Add(sharedMaterial);
                    entity.AddComponent(meshComponent);
                    root.Children.Add(entity);
                    _gridEntities.Add(entity);
                }
            }

        }

        // Sphere
        bool sphere = false;
        if (sphere == true)
        {
            RigidBodyComponent sphereRigidBody = new()
            {
                ColliderShape = new SphereColliderShape(0.5f),
                LinearVelocity = new(0.0f, -5.0f, 0.0f)
            };

            Entity sphereEntity = new("Sphere", new Vector3(0.0f, 5.0f, 0.0f));

            Mesh sphereMesh = ToDispose(Mesh.CreateSphere(0.5f));
            sphereMesh.CreateGpuData(GraphicsDevice);

            MeshComponent meshComponent = new(sphereMesh);
            sphereEntity.AddComponent(meshComponent);
            sphereEntity.AddComponent(sphereRigidBody);

            root.Children.Add(sphereEntity);
        }

        Scene.RootEntity = root;
    }

    public IServiceRegistry Services { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }

    public override void Update(GameTime time)
    {
        float deltaTime = (float)time.Elapsed.TotalSeconds;
        _cubeEntity.Transform.Rotate(10 * deltaTime, 20 * deltaTime, 30 * deltaTime);

        int index = 0;
        for (int x = 0; x < xCount; x++)
        {
            for (int y = 0; y < yCount; y++)
            {
                _gridEntities[index++].Transform.Rotate(10 * deltaTime, 20 * deltaTime, 1);
            }
        }
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        //_cubeEntity.GetComponent<RigidBodyComponent>().LinearVelocity = new Vector3(100, -100.0f, 0.0f);
    }
}
