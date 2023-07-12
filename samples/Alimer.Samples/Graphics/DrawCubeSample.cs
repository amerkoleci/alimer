﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Draw Cube")]
public unsafe sealed class DrawCubeSample : GraphicsSampleBase
{
    private GraphicsBuffer _vertexBuffer;
    private GraphicsBuffer _indexBuffer;
    private BindGroupLayout _bindGroupLayout;
    private PipelineLayout _pipelineLayout;
    private Pipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawCubeSample(GraphicsDevice graphicsDevice, AppView mainView)
        : base("Graphics - Draw Cube", graphicsDevice, mainView)
    {
        var data = MeshUtilities.CreateCube(5.0f);
        _vertexBuffer = ToDispose(GraphicsDevice.CreateBuffer(data.Vertices, BufferUsage.Vertex));

        _indexBuffer = ToDispose(GraphicsDevice.CreateBuffer(data.Indices, BufferUsage.Index));

        BindGroupLayoutEntry[] entries = new BindGroupLayoutEntry[1]
        {
            new BindGroupLayoutEntry(DescriptorType.ConstantBuffer, 0, ShaderStages.Vertex),
        };

        BindGroupLayoutDescription bindGroupLayoutDescription = new(entries, "BindGroupLayout");
        _bindGroupLayout = ToDispose(GraphicsDevice.CreateBindGroupLayout(bindGroupLayoutDescription));

        PushConstantRange pushConstantRange = new(0, sizeof(Matrix4x4));

        PipelineLayoutDescription pipelineLayoutDescription = new(new[] { _bindGroupLayout }, new[] { pushConstantRange }, "PipelineLayout");
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
            new VertexBufferLayout(VertexPositionNormalTexture.SizeInBytes, VertexPositionNormalTexture.VertexAttributes)
        };

        RenderPipelineDescription renderPipelineDesc = new(_pipelineLayout, shaderStages, vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
            Label = "RenderPipeline"
        };
        _renderPipeline = ToDispose(GraphicsDevice.CreateRenderPipeline(renderPipelineDesc));

        _clock = Stopwatch.StartNew();
    }

    public override void Draw(RenderContext context, Texture swapChainTexture)
    {
        float time = _clock.ElapsedMilliseconds / 1000.0f;
        Matrix4x4 world = Matrix4x4.CreateRotationX(time) * Matrix4x4.CreateRotationY(time * 2) * Matrix4x4.CreateRotationZ(time * .7f);

        Matrix4x4 view = Matrix4x4.CreateLookAt(new Vector3(0, 0, 25), new Vector3(0, 0, 0), Vector3.UnitY);
        Matrix4x4 projection = Matrix4x4.CreatePerspectiveFieldOfView((float)Math.PI / 4, AspectRatio, 0.1f, 100);
        Matrix4x4 viewProjection = Matrix4x4.Multiply(view, projection);
        Matrix4x4 worldViewProjection = Matrix4x4.Multiply(world, viewProjection);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(MainView.DepthStencilTexture!);
        RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"
        };

        using (context.PushScopedPassPass(backBufferRenderPass))
        {
            context.SetPipeline(_renderPipeline!);
            context.SetPushConstants(0, worldViewProjection);

            context.SetVertexBuffer(0, _vertexBuffer);
            context.SetIndexBuffer(_indexBuffer, IndexType.Uint16);
            context.DrawIndexed(36);
        }
    }
}
