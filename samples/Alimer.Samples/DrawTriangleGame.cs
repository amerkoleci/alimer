// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Runtime.InteropServices;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;
using Alimer.Shaders;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class DrawTriangleGame : GameApplication
{
    private GraphicsBuffer? _vertexBuffer;
    private PipelineLayout? _pipelineLayout;
    private Pipeline? _renderPipeline;

    protected override void Initialize()
    {
        base.Initialize();

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        ReadOnlySpan<VertexPositionColor> vertexData = stackalloc VertexPositionColor[] {
            new(new Vector3(0.0f, 0.5f, 0.5f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, -0.5f, 0.5f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
            new(new Vector3(-0.5f, -0.5f, 0.5f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
        };
        _vertexBuffer = GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex);

        PipelineLayoutDescription pipelineLayoutDescription = new();
        _pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription);

        byte[] vertexShader = Compile("Triangle.hlsl", "vertexMain", "vs_6_5");
        byte[] fragmentShader = Compile("Triangle.hlsl", "fragmentMain", "ps_6_5");

        var shaderStages = new ShaderStageDescription[2]
        {
            new ShaderStageDescription(ShaderStages.Vertex, vertexShader, "vertexMain"),
            new ShaderStageDescription(ShaderStages.Fragment, fragmentShader, "fragmentMain"),
        };

        var renderPipelineDesc = new RenderPipelineDescription(_pipelineLayout, shaderStages)
        {
        };
        _renderPipeline = GraphicsDevice.CreateRenderPipeline(renderPipelineDesc);
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _vertexBuffer!.Dispose();
            _pipelineLayout!.Dispose();
            _renderPipeline!.Dispose();
        }

        base.Dispose(disposing);
    }

    protected override void Draw(AppTime time)
    {
        RenderContext context = GraphicsDevice.BeginRenderContext("Frame");
        Texture? swapChainTexture = context.AcquireSwapChainTexture(MainView.SwapChain!);
        if (swapChainTexture is not null)
        {
            RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
            RenderPassDepthStencilAttachment depthStencilAttachment = new(MainView.DepthStencilTexture!);
            RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
            {
                Label = "BackBuffer"
            };

            using (context.PushScopedPassPass(backBufferRenderPass))
            {
            }
        }

        //GraphicsDevice.Submit(commandBuffer);
        context.Flush(waitForCompletion: false);

        base.Draw(time);
    }

    private static byte[] Compile(string fileName, string entryPoint, string target)
    {
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSource = File.ReadAllText(Path.Combine(shadersPath, fileName));
        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderSource, entryPoint, target);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return result.GetByteCode().ToArray();
    }

    public static void Main()
    {
        using DrawTriangleGame game = new();
        game.Run();
    }
}


[StructLayout(LayoutKind.Sequential, Pack = 4)]
public readonly struct VertexPositionColor
{
    public static unsafe readonly int SizeInBytes = sizeof(VertexPositionColor);

    public VertexPositionColor(in Vector3 position, in Vector4 color)
    {
        Position = position;
        Color = color;
    }

    public readonly Vector3 Position;
    public readonly Vector4 Color;
}
