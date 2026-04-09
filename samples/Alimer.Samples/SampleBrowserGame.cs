// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Text;
using System.Text.Json;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Physics;
using Alimer.Rendering;
using Alimer.Serialization;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class SampleBrowserGame : Game
{
    private SampleBase _runningSample = null!;

    public SampleBrowserGame(GraphicsBackend preferredGraphicsBackend = GraphicsBackend.Default)
        : base(preferredGraphicsBackend)
    {
        GameSystems.Add(new ImGuiSystem(Services));
    }

    protected override void Initialize()
    {
        base.Initialize();

        {
            var testEntity = new Entity("Test Entity");
            CameraComponent camera = testEntity.AddComponent<CameraComponent>();
            camera.Entity!.Transform.Position = new Vector3(0.0f, 0.0f, 5.0f);
            RigidBodyComponent rigidBody = testEntity.AddComponent<RigidBodyComponent>();

            using MemoryStream memoryStream = new();
            using (var serializer = Serializer.CreateJson(memoryStream, true, new JsonWriterOptions() { Indented = true }))
            {
                using ObjectSerializer objectSerializer = serializer.BeginObject();
                testEntity.Serialize(objectSerializer);
            }
            var json = Encoding.UTF8.GetString(memoryStream.ToArray());

            // Deserialize
            {
                var deserializer = Deserializer.CreateJson(json);
                using ObjectDeserializer objectDeserializer = deserializer.BeginObject();
                var testLoadEntity = new Entity();
                testLoadEntity.Deserialize(objectDeserializer);
            }
        }


        // Setup shader system (until we have a proper asset pipeline)
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        ShaderSystem.AddPath(shadersPath);
        ShaderSystem.Compiler = new AssetShaderCompiler(ShaderSystem);

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_runningSample = new HelloWindowSample(Services, MainWindow);
        //_runningSample = new DrawTriangleSample(Services, MainWindow);
        //_runningSample = new DrawIndexedQuadSample(Services, MainWindow);
        _runningSample = new DrawCubeSample(Services, MainWindow);
        //_runningSample = new DrawTexturedCubeSample(Services, MainWindow);
        // _runningSample = new DrawTexturedFromFileCubeSample(Services, MainWindow);
        //_runningSample = new DrawMeshSample(Services, MainWindow);

        // Engine samples (scene)
        //_runningSample = new SceneCubeSample(Services);
        //_runningSample = new ScenePBRRendererSample(Services);

        MainWindow.Title = $"{_runningSample.Name} - {GraphicsDevice.Backend}";
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _runningSample.Dispose();
        }

        base.Dispose(disposing);
    }

    protected override void Update(GameTime time)
    {
        base.Update(time);

        _runningSample.Update(time);
    }

    protected override void Draw(CommandBuffer renderContext, Texture outputTexture, GameTime time)
    {
        _runningSample.Draw(renderContext, outputTexture);

        base.Draw(renderContext, outputTexture, time);
    }

    public static void Main()
    {
        GraphicsBackend preferredGraphicsBackend = GraphicsBackend.Default;

#if !WINDOWS
        preferredGraphicsBackend = GraphicsBackend.Vulkan;
        //preferredGraphicsBackend = GraphicsBackend.Metal;
        //preferredGraphicsBackend = GraphicsBackend.Null;
#endif

        using SampleBrowserGame game = new(preferredGraphicsBackend);
        game.Run();
    }
}
