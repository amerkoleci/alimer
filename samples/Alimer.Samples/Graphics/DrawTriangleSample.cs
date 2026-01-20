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
    private RenderPipeline _renderPipeline;

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

        using ShaderModule vertexShader = CompileShaderModule("Triangle.hlsl", ShaderStages.Vertex, "vertexMain"u8);
        using ShaderModule fragmentShader = CompileShaderModule("Triangle.hlsl", ShaderStages.Fragment, "fragmentMain"u8);

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
        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.SetVertexBuffer(0, _vertexBuffer!);
        renderPassEncoder.SetPipeline(_renderPipeline!);
        renderPassEncoder.Draw(3);
        renderPassEncoder.EndEncoding();
    }
}
