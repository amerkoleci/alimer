// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Graphics;

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

    private readonly BindGroupLayout _bindGroupLayout1;
    private readonly BindGroup _bindGroup1;

    private readonly PipelineLayout _pipelineLayout;
    private readonly RenderPipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawCubeSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Cube", services, mainWindow)
    {
        var data = MeshUtilities.CreateCube(5.0f);

        _vertexBuffer = ToDispose(CreateBuffer(data.Vertices, BufferUsage.Vertex));
        _indexBuffer = ToDispose(CreateBuffer(data.Indices, BufferUsage.Index));

        _constantBuffer0 = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Matrix4x4), BufferUsage.Constant, MemoryType.Upload));
        _constantBuffer1 = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Color), BufferUsage.Constant, MemoryType.Upload));

        _bindGroupLayout0 = ToDispose(GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(), 0, ShaderStages.Vertex)
            ));

        _bindGroup0 = ToDispose(GraphicsDevice.CreateBindGroup(
            _bindGroupLayout0,
            new BindGroupEntry(0, _constantBuffer0)
            ));

        _bindGroupLayout1 = ToDispose(GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(), 0, ShaderStages.Fragment)
            ));

        _bindGroup1 = ToDispose(GraphicsDevice.CreateBindGroup(
            _bindGroupLayout1,
            new BindGroupEntry(0, _constantBuffer1)
            ));

        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(_bindGroupLayout0, _bindGroupLayout1));

        Dictionary<string, string> vsDefines = new() { { "VERTEX_COLOR", "0" } };
        using ShaderModule vertexShader = CompileShaderModule("Cube", ShaderStages.Vertex, "vertexMain"u8, vsDefines);
        using ShaderModule fragmentShader = CompileShaderModule("Cube", ShaderStages.Fragment, "fragmentMain"u8, vsDefines);

        //ShaderStageDescription vertexShader = LoadShader("Cube", ShaderStages.Vertex, "vertexMain");
        //ShaderStageDescription fragmentShader = LoadShader("Cube", ShaderStages.Fragment, "fragmentMain");

        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new(VertexPositionNormalTexture.SizeInBytes, VertexPositionNormalTexture.VertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(_pipelineLayout, vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
            VertexShader = vertexShader,
            FragmentShader = fragmentShader,
            Label = "RenderPipeline"
        };
        _renderPipeline = ToDispose(GraphicsDevice.CreateRenderPipeline(renderPipelineDesc));

        _clock = Stopwatch.StartNew();
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        float time = _clock.ElapsedMilliseconds / 1000.0f;
        Matrix4x4 world = Matrix4x4.CreateRotationX(time) * Matrix4x4.CreateRotationY(time * 2) * Matrix4x4.CreateRotationZ(time * .7f);

        Matrix4x4 view = Matrix4x4.CreateLookAt(new Vector3(0, 0, 25), new Vector3(0, 0, 0), Vector3.UnitY);
        Matrix4x4 projection = Matrix4x4.CreatePerspectiveFieldOfView((float)Math.PI / 4, AspectRatio, 0.1f, 100);
        Matrix4x4 viewProjection = Matrix4x4.Multiply(view, projection);
        Matrix4x4 worldViewProjection = Matrix4x4.Multiply(world, viewProjection);
        _constantBuffer0.SetData(worldViewProjection);

        Color testColor = new(0.0f, 1.0f, 1.0f, 1.0f);
        _constantBuffer1.SetData(testColor);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.SetPipeline(_renderPipeline!);
        renderPassEncoder.SetBindGroup(0, _bindGroup0);
        renderPassEncoder.SetBindGroup(1, _bindGroup1);

        renderPassEncoder.SetVertexBuffer(0, _vertexBuffer);
        renderPassEncoder.SetIndexBuffer(_indexBuffer, IndexFormat.UInt16);
        renderPassEncoder.DrawIndexed(36);
        renderPassEncoder.EndEncoding();
    }
}
