// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Draw Triangle")]
public sealed class DrawTriangleSample : GraphicsSampleBase
{
    private GraphicsBuffer _vertexBuffer;
    private PipelineLayout _pipelineLayout;
    private Pipeline _renderPipeline;

    public DrawTriangleSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Triangle", services, mainWindow)
    {
        ReadOnlySpan<VertexPositionColor> vertexData = [
            new(new Vector3(0.0f, 0.5f, 0.5f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, -0.5f, 0.5f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
            new(new Vector3(-0.5f, -0.5f, 0.5f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
        ];
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex));

        PipelineLayoutDescription pipelineLayoutDescription = new();
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription));

        ShaderStageDescription vertexShader = CompileShader("Triangle.hlsl", "vertexMain", ShaderStages.Vertex);
        ShaderStageDescription fragmentShader = CompileShader("Triangle.hlsl", "fragmentMain", ShaderStages.Fragment);

        var shaderStages = new ShaderStageDescription[2]
        {
            vertexShader,
            fragmentShader,
        };

        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionColor.SizeInBytes, VertexPositionColor.VertexAttributes)
        };

        RenderPipelineDescription renderPipelineDesc = new(_pipelineLayout, shaderStages, vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
        };
        _renderPipeline = ToDispose(GraphicsDevice.CreateRenderPipeline(renderPipelineDesc));
    }

    public override void Draw(RenderContext context, Texture swapChainTexture)
    {
        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
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
}
