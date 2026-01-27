// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Samples.Graphics;

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

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_runningSample = new HelloWindowSample(Services, MainWindow);
        //_runningSample = new DrawTriangleSample(Services, MainWindow);
        //_runningSample = new DrawIndexedQuadSample(Services, MainWindow);
        //_runningSample = new DrawCubeSample(Services, MainWindow);
        //_runningSample = new DrawTexturedCubeSample(Services, MainWindow);
        _runningSample = new DrawTexturedFromFileCubeSample(Services, MainWindow);
        //_runningSample = new DrawMeshSample(Services, MainWindow);

        // Engine samples (scene)
        //_runningSample = new SceneCubeSample(Services);

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

        // D3D12 is broken ATM
#if !WINDOWS
        //preferredGraphicsBackend = GraphicsBackendType.WebGPU;
        //preferredGraphicsBackend = GraphicsBackend.Vulkan;
        //preferredGraphicsBackend = GraphicsBackendType.Metal;
#endif

        using SampleBrowserGame game = new(preferredGraphicsBackend);
        game.Run();
    }
}
