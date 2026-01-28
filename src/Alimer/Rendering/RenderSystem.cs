// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

public sealed unsafe partial class RenderSystem : EntitySystem<MeshComponent>
{
    private float _elapsedTime;

    private readonly Texture _blackTexture;
    private readonly Texture _whiteTexture;
    private readonly Texture _defaultNormalTexture;
    private readonly Texture _checkerTexture;
    //private readonly Sampler _defaultSampler;

    // TODO: Max frames inflight
    // TODO: Material system

    private readonly ConstantBuffer<FrameConstants> _frameBuffer;
    private readonly BindGroupLayout _frameBindGroupLayout;
    private readonly BindGroup _frameBindGroup;

    private readonly ConstantBuffer<PerViewData> _viewBuffer;
    private readonly BindGroupLayout _viewBindGroupLayout;
    private readonly BindGroup _viewBindGroup;

    private readonly PhysicallyBasedMaterial _defaultMaterial = new();
    private readonly ConstantBuffer<PBRMaterialData> _materialBuffer;
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
        //_defaultSampler = GraphicsDevice.CreateSampler(SamplerDescriptor.LinearWrap);

        ColorFormat = MainWindow.SwapChain!.ColorFormat;
        DepthStencilFormat = PixelFormat.Depth24UnormStencil8;
        SampleCount = TextureSampleCount.Count1; // 4u
        ResolutionMultiplier = 1;

        // WIP code
        // Implement ring buffer
        _frameBuffer = new(GraphicsDevice, label: "Frame Constant Buffer");
        _viewBuffer = new(GraphicsDevice, label: "View Constant Buffer");
        _materialBuffer = new(GraphicsDevice, label: "Material Constant Buffer");

        // Till we have a material system, create a basic pipeline
        _frameBindGroupLayout = GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
            );

        _viewBindGroupLayout = GraphicsDevice.CreateBindGroupLayout(
           new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
           );

        _materialBindGroupLayout = GraphicsDevice.CreateBindGroupLayout(
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Fragment),
            new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment),
            //new BindGroupLayoutEntry(SamplerDescriptor.Default, 0, ShaderStages.Fragment),
            // Static samplers
            new BindGroupLayoutEntry(SamplerDescriptor.PointClamp, 100, ShaderStages.All),          // SamplerPointClamp
            new BindGroupLayoutEntry(SamplerDescriptor.PointWrap, 101, ShaderStages.All),           // SamplerPointWrap
            new BindGroupLayoutEntry(SamplerDescriptor.PointMirror, 102, ShaderStages.All),         // SamplerPointMirror
            new BindGroupLayoutEntry(SamplerDescriptor.LinearClamp, 103, ShaderStages.All),         // SamplerLinearClamp
            new BindGroupLayoutEntry(SamplerDescriptor.LinearWrap, 104, ShaderStages.All),          // SamplerLinearWrap
            new BindGroupLayoutEntry(SamplerDescriptor.LinearMirror, 105, ShaderStages.All),        // SamplerLinearMirror
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicClamp, 106, ShaderStages.All),    // SamplerAnisotropicClamp
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicWrap, 107, ShaderStages.All),     // SamplerAnisotropicWrap
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicMirror, 108, ShaderStages.All),   // SamplerAnisotropicMirror
            new BindGroupLayoutEntry(SamplerDescriptor.ComparisonDepth, 109, ShaderStages.All)    // SamplerAnisotropicMirror
            );

        Span<BindGroupLayout> bindGroupLayouts = [_materialBindGroupLayout, _viewBindGroupLayout, _frameBindGroupLayout];
        Span<PushConstantRange> pushConstantRanges = [
            new PushConstantRange(999, (uint)sizeof(DrawData))
        ];

        PipelineLayoutDescriptor pipelineLayoutDescriptor = new(bindGroupLayouts, pushConstantRanges);

        _pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescriptor);

        // Bind groups with default textures/samplers
        _frameBindGroup = _frameBindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _frameBuffer.Buffer)
            );

        _viewBindGroup = _frameBindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _viewBuffer.Buffer)
            );

        _materialBindGroup = _materialBindGroupLayout.CreateBindGroup(
              new BindGroupEntry(0, _materialBuffer.Buffer),
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
            //_defaultSampler.Dispose();

            // Material
            _materialBuffer.Dispose();
            _materialBindGroupLayout.Dispose();
            _materialBindGroup.Dispose();

            // View
            _viewBuffer.Dispose();
            _viewBindGroup.Dispose();
            _viewBindGroupLayout.Dispose();

            // Frame
            _frameBuffer.Dispose();
            _frameBindGroupLayout.Dispose();
            _frameBindGroup.Dispose();

            _pipelineLayout.Dispose();
            _renderPipeline.Dispose();
        }

        base.Dispose(disposing);
    }

    public override void Update(GameTime time)
    {
        _elapsedTime = (float)time.Total.TotalSeconds;
    }

    public override void Draw(CommandBuffer commandBuffer, Texture outputTexture, GameTime time)
    {
        Render(commandBuffer, outputTexture, Scene.CurrentCamera!, time);
    }

    public void Render(CommandBuffer commandBuffer, Texture output, CameraComponent camera, GameTime time)
    {
        UpdateFrame(time);
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
        renderPass.SetBindGroup(2, _frameBindGroup);

        // For each camera
        renderPass.SetBindGroup(1, _viewBindGroup);

        foreach (MeshComponent meshComponent in Components)
        {
            renderPass.SetBindGroup(0, _materialBindGroup);
            TransformComponent transformComponent = meshComponent.Entity!.Transform;

            DrawData drawData = new()
            {
                WorldMatrix = transformComponent.WorldMatrix
            };

            renderPass.SetPushConstants(0, drawData);
            Mesh? mesh = meshComponent.Mesh;
            if (mesh == null)
                throw new InvalidOperationException("MeshComponent has no Mesh assigned.");

            // Material
            PhysicallyBasedMaterial material = meshComponent.Materials.Count > 0 ? (PhysicallyBasedMaterial)meshComponent.Materials[0] : _defaultMaterial;
            //material.Update(_materialBuffer);
            PBRMaterialData materialData = new();
            materialData.baseColorFactor = material.BaseColorFactor;
            _materialBuffer.SetData(materialData);

            //foreach (var subMesh in mesh.SubMeshes)
            //{
            //    renderPass.SetVertexBuffer(0, mesh.VertexBuffer);
            //    renderPass.SetIndexBuffer(mesh.IndexBuffer, IndexFormat.UInt32);
            //    renderPass.DrawIndexed();
            //}
            mesh.Draw(renderPass, 1u);
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

    private void UpdateFrame(GameTime gameTime)
    {
        FrameConstants frameData = new()
        {
            ElapsedTime = (float)gameTime.Elapsed.TotalSeconds,
            TotalTime = (float)gameTime.Total.TotalSeconds
        };

        _frameBuffer.SetData(frameData);
    }

    private void UpdateCamera(CameraComponent camera)
    {
        PerViewData viewData = new()
        {
            viewMatrix = camera.ViewMatrix,
            projectionMatrix = camera.ViewProjectionMatrix,
            viewProjectionMatrix = camera.ViewProjectionMatrix
        };
        _ = Matrix4x4.Invert(viewData.viewProjectionMatrix, out viewData.inverseViewMatrix);
        _ = Matrix4x4.Invert(viewData.projectionMatrix, out viewData.inverseProjectionMatrix);

        // https://github.com/Aminator/DirectX12GameEngine/blob/master/DirectX12GameEngine.Rendering/Materials/MaterialAttributes.cs
        viewData.cameraPosition = camera.Entity!.Transform.Position;
        //viewData.time = _elapsedTime;

        _viewBuffer.SetData(viewData);
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

    public struct DrawData
    {
        public Matrix4x4 WorldMatrix;
        public uint MaterialIndex;
    }
}
