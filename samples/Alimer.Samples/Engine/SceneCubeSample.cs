// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;
using Alimer.Rendering;

namespace Alimer.Samples.Engime;

[Description("Engine - Scene Cube")]
public sealed class SceneCubeSample : SampleBase
{
    private readonly Entity _cameraEntity;
    private readonly Entity _meshEntity;

    public SceneCubeSample(GraphicsDevice graphicsDevice, SceneSystem sceneSystem)
        : base("Engine - Scene Cube")
    {
        Entity root = new();

        // Camera
        _cameraEntity = new();
        _ = _cameraEntity.Add<CameraComponent>();

        // Mesh
        _meshEntity = new();
        MeshComponent meshComponent = _meshEntity.Add<MeshComponent>();
        meshComponent.Mesh = new Mesh(graphicsDevice);

        root.Children.Add(_cameraEntity);
        root.Children.Add(_meshEntity);

        sceneSystem.RootEntity = root;
    }

    public override void Draw(RenderContext context, Texture swapChainTexture)
    {
    }
}
