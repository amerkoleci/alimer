// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Draw Textured Cube")]
public unsafe sealed class DrawTexturedCubeSample : GraphicsSampleBase
{
    private readonly GraphicsBuffer _vertexBuffer;
    private readonly GraphicsBuffer _indexBuffer;
    private readonly GraphicsBuffer _constantBuffer;
    private readonly Texture _texture;
    private readonly Sampler _sampler;

    private readonly BindGroupLayout _bindGroupLayout;
    private readonly BindGroup _bindGroup;

    private readonly BindGroupLayout _materialBindGroupLayout;
    private readonly BindGroup _materialBindGroup;

    private readonly PipelineLayout _pipelineLayout;
    private readonly Pipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawTexturedCubeSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Textured Cube", services, mainWindow)
    {
        var data = MeshUtilities.CreateCube(5.0f);
        _vertexBuffer = ToDispose(CreateBuffer(data.Vertices, BufferUsage.Vertex));
        _indexBuffer = ToDispose(CreateBuffer(data.Indices, BufferUsage.Index));

        _constantBuffer = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Matrix4x4), BufferUsage.Constant, CpuAccessMode.Write));

        ReadOnlySpan<uint> pixels = [
            0xFFFFFFFF,
            0x00000000,
            0xFFFFFFFF,
            0x00000000,
            0x00000000,
            0xFFFFFFFF,
            0x00000000,
            0xFFFFFFFF,
            0xFFFFFFFF,
            0x00000000,
            0xFFFFFFFF,
            0x00000000,
            0x00000000,
            0xFFFFFFFF,
            0x00000000,
            0xFFFFFFFF,
        ];
        _texture = ToDispose(GraphicsDevice.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 4, 4));

        _sampler = ToDispose(GraphicsDevice.CreateSampler(new SamplerDescriptor()));

        _bindGroupLayout = ToDispose(GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
            ));

        _bindGroup = ToDispose(GraphicsDevice.CreateBindGroup(_bindGroupLayout, new BindGroupEntry(0, _constantBuffer)));

        // Material
        _materialBindGroupLayout = ToDispose(GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment),
            new BindGroupLayoutEntry(new SamplerBindingLayout(), 0, ShaderStages.Fragment) //new BindGroupLayoutEntry(SamplerDescription.PointClamp, 0, ShaderStage.Fragment)
            ));
        _materialBindGroup = ToDispose(GraphicsDevice.CreateBindGroup(_materialBindGroupLayout,
            new BindGroupEntry(0, _texture),
            new BindGroupEntry(0, _sampler))
            );

        //PushConstantRange pushConstantRange = new(0, sizeof(Matrix4x4));
        //PipelineLayoutDescription pipelineLayoutDescription = new(new[] { _bindGroupLayout }, new[] { pushConstantRange }, "PipelineLayout");
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(_bindGroupLayout, _materialBindGroupLayout));

        ShaderStageDescription vertexShader = CompileShader("TexturedCube.hlsl", "vertexMain", ShaderStages.Vertex);
        ShaderStageDescription fragmentShader = CompileShader("TexturedCube.hlsl", "fragmentMain", ShaderStages.Fragment);

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
        _constantBuffer.SetData(worldViewProjection);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color4(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"
        };

        using (context.PushScopedPassPass(backBufferRenderPass))
        {
            context.SetPipeline(_renderPipeline!);
            context.SetBindGroup(0, _bindGroup);
            context.SetBindGroup(1, _materialBindGroup);
            //context.SetPushConstants(0, worldViewProjection);

            context.SetVertexBuffer(0, _vertexBuffer);
            context.SetIndexBuffer(_indexBuffer, IndexType.Uint16);
            context.DrawIndexed(36);
        }
    }
}
