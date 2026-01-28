// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

public sealed unsafe class RenderSystem : EntitySystem<MeshComponent>
{
    private readonly Texture _blackTexture;
    private readonly Texture _whiteTexture;
    private readonly Texture _defaultNormalTexture;
    private readonly Texture _checkerTexture;
    private readonly Sampler _defaultSampler;

    // TODO: Max frames inflight
    private readonly GraphicsBuffer _perViewBuffer;

    // TODO: Material system
    private readonly BindGroupLayout _frameBindGroupLayout;
    private readonly BindGroup _frameBindGroup;
    private readonly BindGroupLayout _materialBindGroupLayout;
    private readonly BindGroup _materialBindGroup;
    private readonly PipelineLayout _pipelineLayout;
    private readonly RenderPipeline _renderPipeline;

    public RenderSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Services = services;
        GraphicsDevice = services.GetService<GraphicsDevice>();
        Scene = services.GetService<SceneSystem>();
        ShaderSystem = services.GetService<ShaderSystem>();
        MainWindow = services.GetService<Window>();

        _blackTexture = CreateTextureFromColor(Colors.Transparent);
        _whiteTexture = CreateTextureFromColor(Colors.White);
        _defaultNormalTexture = CreateTextureFromColor(new Color(0.5f, 0.5f, 1.0f, 0f));
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
        _checkerTexture = GraphicsDevice.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 4, 4);
        _defaultSampler = GraphicsDevice.CreateSampler(SamplerDescriptor.LinearWrap);

        ColorFormat = MainWindow.SwapChain!.ColorFormat;
        DepthStencilFormat = PixelFormat.Depth24UnormStencil8;
        SampleCount = TextureSampleCount.Count1; // 4u
        ResolutionMultiplier = 1;

        // WIP code
        _perViewBuffer = GraphicsDevice.CreateBuffer((ulong)sizeof(PerViewData), BufferUsage.Constant, MemoryType.Upload);

        // Till we have a material system, create a basic pipeline
        _frameBindGroupLayout = GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
            );

        _materialBindGroupLayout = GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment),
            new BindGroupLayoutEntry(SamplerDescriptor.Default, 0, ShaderStages.Fragment)
            );

        ReadOnlySpan<BindGroupLayout> bindGroupLayouts = [_frameBindGroupLayout, _materialBindGroupLayout];
        ReadOnlySpan<PushConstantRange> pushConstantRanges = [
            new PushConstantRange(0, (uint)sizeof(DrawData))
        ];

        PipelineLayoutDescriptor pipelineLayoutDescriptor = new(bindGroupLayouts, pushConstantRanges);

        _pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescriptor);

        // Bind groups with default textures/samplers
        _frameBindGroup = _frameBindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _perViewBuffer)
            );

        _materialBindGroup = _materialBindGroupLayout.CreateBindGroup(
           new BindGroupEntry(0, _checkerTexture.DefaultView!)
           );

        ShaderModule vertexShader = ShaderSystem.GetShaderModule("PBR", ShaderStages.Vertex);
        ShaderModule fragmentShader = ShaderSystem.GetShaderModule("PBR", ShaderStages.Fragment);

        var vertexBufferLayout = new VertexBufferLayout[1]
        {
            new VertexBufferLayout(VertexPositionNormalTexture.SizeInBytes, VertexPositionNormalTexture.RHIVertexAttributes)
        };

        RenderPipelineDescriptor renderPipelineDesc = new(_pipelineLayout, vertexBufferLayout, [ColorFormat], DepthStencilFormat)
        {
            Label = "RenderPipeline",
            VertexShader = vertexShader,
            FragmentShader = fragmentShader
        };
        _renderPipeline = GraphicsDevice.CreateRenderPipeline(renderPipelineDesc);

        MainWindow.SizeChanged += OnCanvasSizeChanged;
        Resize(MainWindow.ClientSize.Width, MainWindow.ClientSize.Height);
    }

    public IServiceRegistry Services { get; }

    public GraphicsDevice GraphicsDevice { get; }
    public SceneSystem Scene { get; }
    public Window MainWindow { get; }

    public PixelFormat ColorFormat { get; }
    public PixelFormat DepthStencilFormat { get; }
    public TextureSampleCount SampleCount { get; }
    public int ResolutionMultiplier { get; } = 1;
    public int Width { get; private set; }
    public int Height { get; private set; }

    public bool ShadowsEnabled { get; set; } = true;
    public int ShadowResolutionMultiplier { get; set; } = 1;

    // Bloom
    public bool BloomEnabled { get; set; } = true;

    public Texture? MultisampleColorTexture { get; private set; }
    public Texture? DepthStencilTexture { get; private set; }

    public ShaderSystem ShaderSystem { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            MultisampleColorTexture?.Dispose();
            DepthStencilTexture?.Dispose();

            _blackTexture.Dispose();
            _whiteTexture.Dispose();
            _defaultNormalTexture.Dispose();
            _checkerTexture.Dispose();
            _defaultSampler.Dispose();

            _perViewBuffer.Dispose();

            _frameBindGroupLayout.Dispose();
            _frameBindGroup.Dispose();
            _materialBindGroupLayout.Dispose();
            _materialBindGroup.Dispose();
            _pipelineLayout.Dispose();
            _renderPipeline.Dispose();
        }

        base.Dispose(disposing);
    }

    public override void Draw(CommandBuffer commandBuffer, Texture outputTexture, GameTime time)
    {
        Render(commandBuffer, outputTexture, Scene.CurrentCamera!);
    }

    public void Render(CommandBuffer commandBuffer, Texture output, CameraComponent camera)
    {
        UpdateCamera(camera);

        RenderPassColorAttachment colorAttachment = new(output.DefaultView!, Colors.Black)
        {
            LoadAction = LoadAction.Clear,
            StoreAction = MultisampleColorTexture != null ? StoreAction.Discard : StoreAction.Store,
            ClearColor = new Color(0.3f, 0.3f, 0.3f)
        };
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor renderPassDescriptor = new(depthStencilAttachment, colorAttachment)
        {
            Label = "Forward render pass"u8
        };

        RenderPassEncoder renderPass = commandBuffer.BeginRenderPass(renderPassDescriptor);
        //renderContext.SetBindGroup(0, camera.bindGroup);

        renderPass.SetPipeline(_renderPipeline!);
        renderPass.SetBindGroup(0, _frameBindGroup);
        renderPass.SetBindGroup(1, _materialBindGroup);

        foreach (MeshComponent meshComponent in Components)
        {
            TransformComponent transformComponent = meshComponent.Entity.Transform;

            DrawData drawData = new()
            {
                WorldMatrix = transformComponent.WorldMatrix
            };

            renderPass.SetPushConstants(0, drawData);
            Mesh? mesh = meshComponent.Mesh;
            if (mesh != null)
            {
                //foreach (var subMesh in mesh.SubMeshes)
                //{
                //    renderPass.SetVertexBuffer(0, mesh.VertexBuffer);
                //    renderPass.SetIndexBuffer(mesh.IndexBuffer, IndexFormat.UInt32);
                //    renderPass.DrawIndexed();
                //}
                mesh.Draw(renderPass, 1u);
            }
        }

        //_cubeMesh.Draw(renderPassEncoder);

        // Loop through all the renderable entities and store them by pipeline.
        //for (const pipeline of this.renderBatch.sortedPipelines) {
        //    passEncoder.setPipeline(pipeline.pipeline);
        //}
        renderPass.EndEncoding();
    }

    public void Resize(int pixelWidth, int pixelHeight)
    {
        Width = pixelWidth * ResolutionMultiplier;
        Height = pixelHeight * ResolutionMultiplier;

        if (SampleCount > TextureSampleCount.Count1)
        {
            MultisampleColorTexture?.Dispose();

            TextureDescriptor multisampleTextureDesc = TextureDescriptor.Texture2D(ColorFormat, (uint)Width, (uint)Height, 1, 1, TextureUsage.RenderTarget, SampleCount);
            MultisampleColorTexture = GraphicsDevice.CreateTexture(in multisampleTextureDesc);
        }

        if (DepthStencilFormat != PixelFormat.Undefined &&
            DepthStencilFormat.IsDepthStencilFormat())
        {
            DepthStencilTexture?.Dispose();

            TextureDescriptor depthStencilTextureDesc = TextureDescriptor.Texture2D(DepthStencilFormat, (uint)Width, (uint)Height, 1, 1,
                usage: TextureUsage.RenderTarget,
                sampleCount: SampleCount
                );
            DepthStencilTexture = GraphicsDevice.CreateTexture(in depthStencilTextureDesc);
        }
    }

    private void OnCanvasSizeChanged(object? sender, EventArgs e)
    {
        Resize(MainWindow.ClientSize.Width, MainWindow.ClientSize.Height);
    }

    private void UpdateCamera(CameraComponent camera)
    {
        PerViewData perViewData = new PerViewData
        {
            viewMatrix = camera.ViewMatrix,
            projectionMatrix = camera.ViewProjectionMatrix,
            viewProjectionMatrix = camera.ViewProjectionMatrix
        };

        _perViewBuffer.SetData(perViewData);
    }

    private Texture CreateTextureFromColor(in Color color)
    {
        ReadOnlySpan<uint> pixels = [color.ToRgba()];
        return GraphicsDevice.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 1, 1);
    }

    protected override void OnEntityComponentAdded(MeshComponent component)
    {

    }

    protected override void OnEntityComponentRemoved(MeshComponent component)
    {

    }

    // Must match shader layout (ShaderTypes.h)
    public struct PerViewData
    {
        public Matrix4x4 viewMatrix;
        public Matrix4x4 projectionMatrix;
        public Matrix4x4 viewProjectionMatrix;
        public Matrix4x4 inverseViewMatrix;
        public Matrix4x4 inverseProjectionMatrix;
        public Vector3 cameraPosition;
        public float time;
    }

    public struct DrawData
    {
        public Matrix4x4 WorldMatrix;
        public uint MaterialIndex;
    }
}
