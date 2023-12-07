// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - DrawIndexed Quad")]
public sealed class DrawIndexedQuadSample : GraphicsSampleBase
{
    private GraphicsBuffer _vertexBuffer;
    private GraphicsBuffer _indexBuffer;
    private PipelineLayout _pipelineLayout;
    private Pipeline _renderPipeline;

    public DrawIndexedQuadSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - DrawIndexed Quad", services, mainWindow)
    {
        ReadOnlySpan<VertexPositionColor> vertexData = [
            new(new Vector3(-0.5f, 0.5f, 0.5f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, 0.5f, 0.5f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, -0.5f, 0.5f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
            new(new Vector3(-0.5f, -0.5f, 0.5f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
        ];
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex));

        ReadOnlySpan<ushort> indexData = stackalloc ushort[6] { 0, 1, 2, 0, 2, 3 };
        _indexBuffer = ToDispose(GraphicsDevice.CreateBuffer(indexData, BufferUsage.Index));

        PipelineLayoutDescription pipelineLayoutDescription = new()
        {
            Label = "PipelineLayout"
        };
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription));

        ShaderStageDescription vertexShader = CompileShader("Triangle.hlsl", "vertexMain", ShaderStage.Vertex);
        ShaderStageDescription fragmentShader = CompileShader("Triangle.hlsl", "fragmentMain", ShaderStage.Fragment);

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
            Label = "RenderPipeline"
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
            context.SetVertexBuffer(0, _vertexBuffer);
            context.SetIndexBuffer(_indexBuffer, IndexType.Uint16);
            context.SetPipeline(_renderPipeline!);
            context.DrawIndexed(6);
        }
    }
}
