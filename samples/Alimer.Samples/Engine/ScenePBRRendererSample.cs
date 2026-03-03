// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Assets.Graphics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Input;
using Alimer.Physics;
using Alimer.Rendering;

namespace Alimer.Samples;

[Description("Engine - Scene PBR Renderer")]
public sealed class ScenePBRRendererSample : SampleBase
{
    private readonly Entity _cameraEntity;
    private readonly Entity _damagedHelmetEntity;
    //private readonly Texture _environmentMap;

    public ScenePBRRendererSample(IServiceRegistry services)
        : base("Engine - Scene Cube")
    {
        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        Input = services.GetService<InputManager>();
        Entity root = new();

        // Camera
        _cameraEntity = new();
        CameraComponent camera = _cameraEntity.AddComponent<CameraComponent>();
        camera.Entity!.Transform.Position = new Vector3(0.0f, 0.0f, 5.0f);
        root.Children.Add(_cameraEntity);

        {
            // GLTF mesh
            string meshesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Meshes");
            //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
            //_environmentMap = ToDispose(Texture.FromFile(GraphicsDevice, Path.Combine(texturesPath, "zavelstein_ibl.ktx")));

            MeshImporter meshImporter = new(GraphicsDevice);
            MeshMetadata meshMetadata = new()
            {
                FileFullPath = Path.Combine(meshesPath, "DamagedHelmet.glb")
            };
            MeshAsset meshAsset = meshImporter.Import(meshMetadata).Result;

            Mesh damagedHelmetMesh = ToDispose(meshAsset.Mesh);
            _damagedHelmetEntity = new("Damaged Helmet", meshAsset.Translation);
            _damagedHelmetEntity.Transform.Rotation = meshAsset.Rotation;
            _damagedHelmetEntity.Transform.Scale = meshAsset.Scale;

            MeshComponent meshComponent = new(damagedHelmetMesh);

            foreach (Material material in meshAsset.Materials)
            {
                meshComponent.Materials.Add(ToDispose(material));
            }

            _damagedHelmetEntity.AddComponent(meshComponent);
            root.Children.Add(_damagedHelmetEntity);
        }

        // Lights
        Entity directionalLightEntity = new("DirectionalLight")
        {
            Direction = new(0.5f, 1.0f, 0.25f)
        };
        LightComponent directionalLight = directionalLightEntity.AddComponent<LightComponent>();
        directionalLight.LightType = LightType.Directional;
        directionalLight.Color = Colors.Gray;
        root.Children.Add(directionalLightEntity);

        // Point lights
        int pointLightCount = 3;
        float timestamp = 0.0f;
        for (int i = 0; i < pointLightCount; i++)
        {
            float r = (i / pointLightCount) * MathF.PI * 2 + (timestamp / 1000);

            Entity pointLightEntity = new($"PointLight_{i}")
            {
                Transform =
                {
                    Position = new Vector3(
                        MathF.Sin(r) * 2.5f,
                        MathF.Sin(timestamp / 1000 + (i / pointLightCount)) * 1.5f,
                        MathF.Cos(r) * 2.5f
                        )
                }
            };
            LightComponent pointLight = pointLightEntity.AddComponent<LightComponent>();
            pointLight.LightType = LightType.Point;
            pointLight.Color = Colors.Green;
            root.Children.Add(pointLightEntity);
        }

        Scene.RootEntity = root;
    }

    public IServiceRegistry Services { get; }
    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }
    public InputManager Input { get; }

    public override void Update(GameTime time)
    {
        float deltaTime = (float)time.Elapsed.TotalSeconds;

        // Camera movement
        //if (InputManager.IsMouseButtonDown(MouseButton::Right))
        //{
        //    const Vector2 mouseMove = Input::GetMousePositionDelta();
        //    yaw -= mouseMove.x * 0.1f;
        //    pitch += mouseMove.y * 0.1f;
        //    pitch = Clamp(pitch, -90.0f, 90.0f);
        //    cameraEntity->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0.f));
        //}

        float moveSpeed = (Input.IsKeyDown(Keys.LeftShift) || Input.IsKeyDown(Keys.RightShift)) ? 50.0f : 5.0f;

        if (Input.IsKeyDown(Keys.W))
        {
            _cameraEntity.Translate(Vector3.Forward * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.S))
        {
            _cameraEntity.Translate(Vector3.Backward * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.A))
        {
            _cameraEntity.Translate(Vector3.Left * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.D))
        {
            _cameraEntity.Translate(Vector3.Right * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.PageUp))
        {
            _cameraEntity.Translate(Vector3.Up * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.PageDown))
        {
            _cameraEntity.Translate(Vector3.Down * deltaTime * moveSpeed);
        }

        if (Input.IsKeyDown(Keys.Space))
        {
            //cameraEntity->LookAt(Vector3::Zero);
        }

        //_damagedHelmetEntity.Transform.Rotate(10 * deltaTime, 20 * deltaTime, 30 * deltaTime);
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        //_cubeEntity.GetComponent<RigidBodyComponent>().LinearVelocity = new Vector3(100, -100.0f, 0.0f);
    }
}
