// Copyright (c) Amer Koleci and Contributors.
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
        GameSystems.Add(new ImGuiSystem(Services));
    }

    protected override void Initialize()
    {
        base.Initialize();

        foreach (VertexFormat format in Enum.GetValues<VertexFormat>())
        {
            if (format == VertexFormat.Undefined)
                continue;

            GraphicsDevice.QueryVertexFormatSupport(format);
        }

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_runningSample = new HelloWindowSample(Services, MainWindow);
        //_runningSample = new DrawTriangleSample(Services, MainWindow);
        //_runningSample = new DrawIndexedQuadSample(Services, MainWindow);
        _runningSample = new DrawCubeSample(Services, MainWindow);
        //_runningSample = new DrawTexturedCubeSample(Services, MainWindow);
        //_runningSample = new DrawTexturedFromFileCubeSample(GraphicsDevice, MainWindow);
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

    protected override void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {
        _runningSample.Draw(renderContext, outputTexture);

        base.Draw(renderContext, outputTexture, time);
    }

    public static void Main()
    {
        GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count;

#if !WINDOWS
        //preferredGraphicsBackend = GraphicsBackendType.WebGPU;
        preferredGraphicsBackend = GraphicsBackendType.Vulkan;
#endif

        using SampleBrowserApp game = new(preferredGraphicsBackend);
        game.Run();
    }
}
