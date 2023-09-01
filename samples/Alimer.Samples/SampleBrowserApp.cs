// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Samples.Engime;
using Alimer.Samples.Graphics;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class SampleBrowserApp : GameApplication
{
    private SampleBase _runningSample = null!;

    public SampleBrowserApp(GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count)
        : base(preferredGraphicsBackend)
    {
    }

    protected override void Initialize()
    {
        base.Initialize();

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_runningSample = new HelloWindowSample(GraphicsDevice, MainWindow);
        //_runningSample = new DrawTriangleSample(GraphicsDevice, MainWindow);
        //_runningSample = new DrawIndexedQuadSample(GraphicsDevice, MainWindow);
        //_runningSample = new DrawCubeSample(GraphicsDevice, MainWindow);
        //_runningSample = new DrawTexturedCubeSample(GraphicsDevice, MainWindow);
        _runningSample = new DrawTexturedFromFileCubeSample(GraphicsDevice, MainWindow);
        //_runningSample = new DrawMeshSample(GraphicsDevice, MainWindow);

        // Engine samples (scene)
        //_runningSample = new SceneCubeSample(GraphicsDevice, SceneSystem);

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

    protected override void Draw(AppTime time)
    {
        RenderContext context = GraphicsDevice.BeginRenderContext("Frame");
        Texture? swapChainTexture = MainWindow.SwapChain!.GetCurrentTexture();
        if (swapChainTexture is not null)
        {
            _runningSample.Draw(context, swapChainTexture);
        }

        context.Present(MainWindow.SwapChain!);

        //GraphicsDevice.Submit(commandBuffer);
        context.Flush(waitForCompletion: false);

        base.Draw(time);
    }

    public static void Main()
    {
        GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count;

#if !WINDOWS
        //preferredGraphicsBackend = GraphicsBackendType.WebGPU;
        //preferredGraphicsBackend = GraphicsBackendType.Vulkan;
#endif

        using SampleBrowserApp game = new(preferredGraphicsBackend);
        game.Run();
    }
}
