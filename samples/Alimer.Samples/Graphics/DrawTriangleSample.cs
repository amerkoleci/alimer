// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Samples;

[Description("Graphics - Draw Triangle")]
public sealed class DrawTriangleSample : GraphicsSampleBase
{
    private GPUBuffer _vertexBuffer;
    private RenderPipeline _renderPipeline;

    public DrawTriangleSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Triangle", services, mainWindow)
    {
        ReadOnlySpan<VertexPositionColor> vertexData = [
            new(new Vector3(0.0f, 0.5f, 0.0f), Colors.Red),
            new(new Vector3(-0.5f, -0.5f, 0.0f), Colors.Lime),
            new(new Vector3(0.5f, -0.5f, 0.0f), Colors.Blue),
        ];
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(vertexData, GPUBufferUsage.Vertex));

        using ShaderModule vertexShader = CompileShaderModule("Triangle", ShaderStages.Vertex, "vertexMain");
        using ShaderModule fragmentShader = CompileShaderModule("Triangle", ShaderStages.Fragment, "fragmentMain");

        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionColor.SizeInBytes, VertexPositionColor.VertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(default, vertexBufferLayout, ColorFormats, DepthStencilFormat)
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
