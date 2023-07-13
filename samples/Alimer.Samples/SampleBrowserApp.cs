// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Samples.Graphics;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class SampleBrowserApp : GameApplication
{
    private SampleBase _samplerBase = null!;

    public SampleBrowserApp(GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count)
        : base(preferredGraphicsBackend)
    {
    }

    protected override void Initialize()
    {
        base.Initialize();

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_samplerBase = new DrawTriangleSample(GraphicsDevice, MainView);
        //_samplerBase = new DrawIndexedQuadSample(GraphicsDevice, MainView);
        _samplerBase = new DrawCubeSample(GraphicsDevice, MainWindow);

        MainWindow.Title = $"{_samplerBase.Name} - {GraphicsDevice.Backend}";
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _samplerBase.Dispose();
        }

        base.Dispose(disposing);
    }

    protected override void Draw(AppTime time)
    {
        RenderContext context = GraphicsDevice.BeginRenderContext("Frame");
        Texture? swapChainTexture = context.AcquireSwapChainTexture(MainWindow.SwapChain!);
        if (swapChainTexture is not null)
        {
            if (_samplerBase is GraphicsSampleBase graphicsSampleBase)
            {
                graphicsSampleBase.Draw(context, swapChainTexture);
            }
        }

        //GraphicsDevice.Submit(commandBuffer);
        context.Flush(waitForCompletion: false);

        base.Draw(time);
    }

    public static void Main()
    {
        GraphicsBackendType preferredGraphicsBackend = GraphicsBackendType.Count;

#if !WINDOWS
        //preferredGraphicsBackend = GraphicsBackendType.Vulkan;
#endif

        using SampleBrowserApp game = new(preferredGraphicsBackend);
        game.Run();
    }
}
