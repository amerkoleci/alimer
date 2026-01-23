// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Diagnostics;
using System.Numerics;
using Alimer.Graphics;
using Alimer.Rendering;
using CommunityToolkit.Diagnostics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Draw Textured Cube")]
public unsafe sealed class DrawTexturedCubeSample : GraphicsSampleBase
{
    private readonly Mesh _cubeMesh;
    private readonly GraphicsBuffer _constantBuffer;
    private readonly Texture _texture;
    private readonly Sampler _sampler;

    private readonly BindGroupLayout _bindGroupLayout;
    private readonly BindGroup _bindGroup;

    private readonly BindGroupLayout _materialBindGroupLayout;
    private readonly BindGroup _materialBindGroup;

    private readonly PipelineLayout _pipelineLayout;
    private readonly RenderPipeline _renderPipeline;

    private Stopwatch _clock;

    public DrawTexturedCubeSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Draw Textured Cube", services, mainWindow)
    {
        _cubeMesh = ToDispose(Mesh.CreateCube(5.0f));
        //_cubeMesh = ToDispose(Mesh.CreateSphere(5.0f));
        _cubeMesh.CreateGpuData(GraphicsDevice);

        _constantBuffer = ToDispose(GraphicsDevice.CreateBuffer((ulong)sizeof(Matrix4x4), BufferUsage.Constant, MemoryType.Upload));

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
            new BindGroupEntry(0, _texture.DefaultView!),
            new BindGroupEntry(0, _sampler))
            );

        //PushConstantRange pushConstantRange = new(0, sizeof(Matrix4x4));
        //PipelineLayoutDescription pipelineLayoutDescription = new(new[] { _bindGroupLayout }, new[] { pushConstantRange }, "PipelineLayout");
        _pipelineLayout = ToDispose(GraphicsDevice.CreatePipelineLayout(_bindGroupLayout, _materialBindGroupLayout));

        using ShaderModule vertexShader = CompileShaderModuleNew("TexturedCube", ShaderStages.Vertex, "vertexMain"u8);
        using ShaderModule fragmentShader = CompileShaderModuleNew("TexturedCube", ShaderStages.Fragment, "fragmentMain"u8);


        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionNormalTexture.SizeInBytes, VertexPositionNormalTexture.VertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(_pipelineLayout, vertexBufferLayout, ColorFormats, DepthStencilFormat)
        {
            Label = "RenderPipeline",
            VertexShader = vertexShader,
            FragmentShader = fragmentShader
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
        _constantBuffer.SetData(worldViewProjection);

        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.SetPipeline(_renderPipeline!);
        renderPassEncoder.SetBindGroup(0, _bindGroup);
        renderPassEncoder.SetBindGroup(1, _materialBindGroup);
        //context.SetPushConstants(0, worldViewProjection);

        _cubeMesh.Draw(renderPassEncoder);

        renderPassEncoder.EndEncoding();
    }
}
