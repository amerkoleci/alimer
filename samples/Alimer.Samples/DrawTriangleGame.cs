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

        ShaderStageDescription vertexShader = Compile("Triangle.hlsl", "vertexMain", ShaderStages.Vertex);
        ShaderStageDescription fragmentShader = Compile("Triangle.hlsl", "fragmentMain", ShaderStages.Fragment);

        var shaderStages = new ShaderStageDescription[2]
        {
            vertexShader,
            fragmentShader,
        };

        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionColor.SizeInBytes, VertexPositionColor.VertexAttributes)
        };

        var colorFormats = new PixelFormat[1]
        {
            MainView.SwapChain!.ColorFormat
        };
        var depthStencilFormat = MainView.DepthStencilFormat;

        var renderPipelineDesc = new RenderPipelineDescription(_pipelineLayout, shaderStages, vertexBufferLayout, colorFormats, depthStencilFormat)
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
                context.SetVertexBuffer(0, _vertexBuffer!);
                context.SetPipeline(_renderPipeline!);
                context.Draw(3);
            }
        }

        //GraphicsDevice.Submit(commandBuffer);
        context.Flush(waitForCompletion: false);

        base.Draw(time);
    }

    private ShaderStageDescription Compile(string fileName, string entryPoint, ShaderStages stage)
    {
        ShaderFormat shaderFormat = GraphicsDevice.Backend == GraphicsBackendType.Vulkan ? ShaderFormat.SPIRV : ShaderFormat.DXIL;

        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        string shaderSource = File.ReadAllText(Path.Combine(shadersPath, fileName));

        ShaderStage shaderCompilerStage = ShaderStage.Vertex;
        switch (stage)
        {
            case ShaderStages.Vertex:
                shaderCompilerStage = ShaderStage.Vertex;
                break;

            case ShaderStages.Fragment:
                shaderCompilerStage = ShaderStage.Fragment;
                break;

            case ShaderStages.Compute:
                shaderCompilerStage = ShaderStage.Compute;
                break;

            default:
                throw new NotImplementedException();
        }

        ShaderCompilationOptions options = new()
        {
            ShaderStage = shaderCompilerStage,
            EntryPoint = entryPoint,
        };

        using ShaderCompilationResult result = ShaderCompiler.Instance.Compile(shaderFormat, shaderSource,  in options);
        if (result.Failed)
        {
            throw new GraphicsException(result.ErrorMessage);
        }

        return new ShaderStageDescription(stage, result.GetByteCode().ToArray(), entryPoint);
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
    public static readonly unsafe uint SizeInBytes = (uint)sizeof(VertexPositionColor);

    public static readonly VertexAttribute[] VertexAttributes = new[]
    {
        new VertexAttribute(VertexFormat.Float3, 0, 0),
        new VertexAttribute(VertexFormat.Float4, 12, 1)
    };

    public VertexPositionColor(in Vector3 position, in Vector4 color)
    {
        Position = position;
        Color = color;
    }

    public readonly Vector3 Position;
    public readonly Vector4 Color;
}
