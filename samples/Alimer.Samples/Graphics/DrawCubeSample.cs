﻿// Copyright (c) Amer Koleci and Contributors.
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
    private readonly GraphicsBuffer _vertexBuffer;
    private readonly GraphicsBuffer _indexBuffer;
    private readonly GraphicsBuffer _constantBuffer0;
    private readonly GraphicsBuffer _constantBuffer1;

    private readonly BindGroupLayout _bindGroupLayout0;
    private readonly BindGroup _bindGroup0;
    private readonly PipelineLayout _pipelineLayout;
    private readonly Pipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawCubeSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Cube", services, mainWindow)
    {
        var data = MeshUtilities.CreateCube(5.0f);

        _vertexBuffer = ToDispose(CreateBuffer(data.Vertices, BufferUsage.Vertex));
        _indexBuffer = ToDispose(CreateBuffer(data.Indices, BufferUsage.Index));

        _constantBuffer0 = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Matrix4x4), BufferUsage.Constant, CpuAccessMode.Write));
        _constantBuffer1 = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Color), BufferUsage.Constant, CpuAccessMode.Write));

        _bindGroupLayout0 = ToDispose(GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(), 0, ShaderStage.Vertex),
            new BindGroupLayoutEntry(new BufferBindingLayout(), 1, ShaderStage.Fragment)
            ));

        _bindGroup0 = ToDispose(GraphicsDevice.CreateBindGroup(
            _bindGroupLayout0,
            new BindGroupEntry(0, _constantBuffer0),
            new BindGroupEntry(1, _constantBuffer1)
            ));

        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(_bindGroupLayout0));

        ShaderStageDescription vertexShader = CompileShader("Cube.hlsl", "vertexMain", ShaderStage.Vertex);
        ShaderStageDescription fragmentShader = CompileShader("Cube.hlsl", "fragmentMain", ShaderStage.Fragment);

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
        _constantBuffer0.SetData(worldViewProjection);

        Color testColor = new Color(0.0f, 0.0f, 1.0f, 1.0f);
        _constantBuffer1.SetData(testColor);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"
        };

        using (context.PushScopedPassPass(backBufferRenderPass))
        {
            context.SetPipeline(_renderPipeline!);
            context.SetBindGroup(0, _bindGroup0);

            context.SetVertexBuffer(0, _vertexBuffer);
            context.SetIndexBuffer(_indexBuffer, IndexType.Uint16);
            context.DrawIndexed(36);
        }
    }
}
