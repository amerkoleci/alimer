// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Draw Cube")]
public sealed class DrawCubeSample : GraphicsSampleBase
{
    private GraphicsBuffer _vertexBuffer;
    private GraphicsBuffer _indexBuffer;
    private BindGroupLayout _bindGroupLayout;
    private PipelineLayout _pipelineLayout;
    private Pipeline _renderPipeline;

    public DrawCubeSample(GraphicsDevice graphicsDevice, AppView mainView)
        : base("Graphics - Draw Cube", graphicsDevice, mainView)
    {
        ReadOnlySpan<VertexPositionColor> vertexData = stackalloc VertexPositionColor[] {
            new(new Vector3(-0.5f, 0.5f, 0.5f), new Vector4(1.0f, 0.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, 0.5f, 0.5f), new Vector4(0.0f, 1.0f, 0.0f, 1.0f)),
            new(new Vector3(0.5f, -0.5f, 0.5f), new Vector4(0.0f, 0.0f, 1.0f, 1.0f)),
            new(new Vector3(-0.5f, -0.5f, 0.5f), new Vector4(1.0f, 1.0f, 0.0f, 1.0f)),
        };
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(vertexData, BufferUsage.Vertex));

        ReadOnlySpan<ushort> indexData = stackalloc ushort[6] { 0, 1, 2, 0, 2, 3 };
        _indexBuffer = ToDispose(GraphicsDevice.CreateBuffer(indexData, BufferUsage.Index));

        BindGroupLayoutEntry[] entries = new BindGroupLayoutEntry[1]
        {
            new BindGroupLayoutEntry(DescriptorType.ConstantBuffer, 0, ShaderStages.Vertex),
        };

        BindGroupLayoutDescription bindGroupLayoutDescription = new(entries, "BindGroupLayout");
        _bindGroupLayout = ToDispose(GraphicsDevice.CreateBindGroupLayout(bindGroupLayoutDescription));

        PipelineLayoutDescription pipelineLayoutDescription = new(_bindGroupLayout, "PipelineLayout");
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription));

        ShaderStageDescription vertexShader = CompileShader("Cube.hlsl", "vertexMain", ShaderStages.Vertex);
        ShaderStageDescription fragmentShader = CompileShader("Cube.hlsl", "fragmentMain", ShaderStages.Fragment);

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
        RenderPassDepthStencilAttachment depthStencilAttachment = new(MainView.DepthStencilTexture!);
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
