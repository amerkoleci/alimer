// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - DrawIndexed Quad")]
public sealed class DrawIndexedQuadSample : GraphicsSampleBase
{
    private GraphicsBuffer _vertexBuffer;
    private GraphicsBuffer _indexBuffer;
    private PipelineLayout _pipelineLayout;
    private RenderPipeline _renderPipeline;

    public DrawIndexedQuadSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - DrawIndexed Quad", services, mainWindow)
    {
        ReadOnlySpan<VertexPositionColor> vertexData = [
            new(new Vector3(-0.5f, 0.5f, 0.5f), Colors.Red),
            new(new Vector3(0.5f, 0.5f, 0.5f), Colors.Lime),
            new(new Vector3(0.5f, -0.5f, 0.5f), Colors.Blue),
            new(new Vector3(-0.5f, -0.5f, 0.5f), Colors.Yellow),
        ];
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex));

        ReadOnlySpan<ushort> indexData = [0, 2, 1, 0, 3, 2];
        _indexBuffer = ToDispose(GraphicsDevice.CreateBuffer(indexData, BufferUsage.Index));

        PipelineLayoutDescriptor pipelineLayoutDescription = new()
        {
            Label = "PipelineLayout"
        };
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription));

        using ShaderModule vertexShader = CompileShaderModuleNew("Triangle", ShaderStages.Vertex, "vertexMain"u8);
        using ShaderModule fragmentShader = CompileShaderModuleNew("Triangle", ShaderStages.Fragment, "fragmentMain"u8);


        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionColor.SizeInBytes, VertexPositionColor.VertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(_pipelineLayout, vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
            VertexShader = vertexShader,
            FragmentShader = fragmentShader,
            Label = "RenderPipeline"
        };
        _renderPipeline = ToDispose(GraphicsDevice.CreateRenderPipeline(renderPipelineDesc));
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        RenderPassColorAttachment colorAttachment = new(swapChainTexture.DefaultView!, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!.DefaultView!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.SetVertexBuffer(0, _vertexBuffer);
        renderPassEncoder.SetIndexBuffer(_indexBuffer, IndexFormat.UInt16);
        renderPassEncoder.SetPipeline(_renderPipeline!);
        renderPassEncoder.DrawIndexed(6);
        renderPassEncoder.EndEncoding();
    }
}
