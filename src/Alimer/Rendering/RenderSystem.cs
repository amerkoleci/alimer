// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

// TODO: Add support for RenderableComponent (ParticleSystem or other?)

public sealed partial class RenderSystem : EntitySystem<MeshComponent>
{
    // ShaderTypes.h
    public const int FrameBindGroupSpace = 3;
    public const int ViewBindGroupSpace = 2;
    public const int InstanceBindGroupSpace = 1;
    public const int MaterialBindGroupSpace = 0;

    private readonly Texture _blackTexture;
    private readonly Texture _whiteTexture;
    private readonly Texture _defaultNormalTexture;

    // TODO: Max frames inflight
    // TODO: Material system
    private readonly Dictionary<Type, IGPUMaterialFactory> _gpuMaterialFactories = [];
    //private readonly Dictionary<Mesh, List<GPUInstanceData>> _renderMeshInstances = [];
    private readonly RenderBatch _renderBatch;

    private readonly ConstantBuffer<FrameConstants> _frameBuffer;
    private readonly BindGroup _frameBindGroup;

    private readonly ConstantBuffer<PerViewData> _viewBuffer;
    private readonly BindGroup _viewBindGroup;

    private readonly UnlitMaterial _defaultMaterial = new();

    public RenderSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Services = services;
        Device = services.GetService<GraphicsDevice>();
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
        CheckerTexture = ToDispose(Device.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 4, 4));
        DefaultSampler = ToDispose(Device.CreateSampler(SamplerDescriptor.PointWrap));

        ColorFormat = MainWindow.SwapChain!.ColorFormat;
        DepthStencilFormat = PixelFormat.Depth24UnormStencil8;
        SampleCount = TextureSampleCount.Count1; // 4u
        ResolutionMultiplier = 1;
        GPUMaterialFactories.RegisterDefault();

        _renderBatch = ToDispose(new RenderBatch(Device));

        // WIP code
        // Implement ring buffer
        _frameBuffer = new(Device, label: "Frame Constant Buffer");
        _viewBuffer = new(Device, label: "View Constant Buffer");

        // Till we have a material system, create a basic pipeline
        FrameBindGroupLayout = ToDispose(Device.CreateBindGroupLayout(
            "Frame BindGroupLayout",
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
            ));

        ViewBindGroupLayout = ToDispose(Device.CreateBindGroupLayout(
           new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Vertex)
           ));

        // Bind groups with default textures/samplers
        _frameBindGroup = FrameBindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _frameBuffer.Buffer)
            );

        _viewBindGroup = FrameBindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _viewBuffer.Buffer)
            );

        MainWindow.SizeChanged += OnCanvasSizeChanged;
        Resize(MainWindow.ClientSize.Width, MainWindow.ClientSize.Height);
    }

    public IServiceRegistry Services { get; }

    public GraphicsDevice Device { get; }
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
    public Texture CheckerTexture { get; }
    public Sampler DefaultSampler { get; }

    public BindGroupLayout InstanceBindGroupLayout => _renderBatch.InstanceBindGroupLayout; // 1
    public BindGroupLayout ViewBindGroupLayout { get; } // 2
    public BindGroupLayout FrameBindGroupLayout { get; } // 3

    public ShaderSystem ShaderSystem { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            // Dispose all material factories
            foreach (IGPUMaterialFactory factory in _gpuMaterialFactories.Values)
            {
                factory.Dispose();
            }
            _gpuMaterialFactories.Clear();

            MultisampleColorTexture?.Dispose();
            DepthStencilTexture?.Dispose();

            _blackTexture.Dispose();
            _whiteTexture.Dispose();
            _defaultNormalTexture.Dispose();

            // View
            _viewBuffer.Dispose();
            _viewBindGroup.Dispose();

            // Frame
            _frameBuffer.Dispose();
            _frameBindGroup.Dispose();
        }

        base.Dispose(disposing);
    }

    public override void Update(GameTime time)
    {
        if (Scene.CurrentCamera is null)
            return;

        // Prepare frame instances (visible to camera)
        BoundingFrustum cameraFrustum = Scene.CurrentCamera.Frustum;

        foreach (MeshComponent meshComponent in Components)
        {
            if (meshComponent.Mesh is null)
                continue;

            // Check if visible to camera frustum before drawing
            BoundingBox worldBoundingBox = meshComponent.WorldBoundingBox;
            ContainmentType containmentType = cameraFrustum.Contains(worldBoundingBox);
            if (containmentType == ContainmentType.Disjoint)
                continue;

            Mesh mesh = meshComponent.Mesh;
            //MeshVertexBufferLayout layout = component.Mesh.VertexBufferLayout;
            VertexBufferLayout gpuLayout = new(mesh.VertexStride, mesh.VertexAttributes);
            Span<VertexBufferLayout> geometryLayout = [gpuLayout];

            foreach (SubMesh subMesh in mesh.SubMeshes)
            {
                int materialIndex = subMesh.MaterialIndex;
                if (materialIndex >= meshComponent.Materials.Count)
                    continue;

                Material material = meshComponent.Materials[materialIndex];
                if (!_gpuMaterialFactories.TryGetValue(material.GetType(), out IGPUMaterialFactory? factory))
                {
                    throw new InvalidOperationException($"No GPU material factory found for material type '{material.GetType()}'.");
                }

                Matrix4x4 worldMatrix = meshComponent.Entity!.WorldTransform;

                GPUInstanceData instanceData = new(worldMatrix, (uint)materialIndex, Count: 1);

                bool skinned = false;
                GPURenderPipeline pipeline = factory.GetPipeline(geometryLayout, material, skinned);
                //GPUMaterialBindGroups bindGroups = new(factory.GetBindGroup(material, skinned));
                BindGroup bindGroup = factory.GetBindGroup(material, skinned);
                //RenderPrimitive primitive = new(subMesh, pipeline, bindGroups);
                _renderBatch.AddRenderable(subMesh, pipeline, bindGroup, instanceData);
            }
        }
    }

    public override void Draw(CommandBuffer commandBuffer, Texture outputTexture, GameTime time)
    {
        if (Scene.CurrentCamera is null)
            return;

        Render(commandBuffer, outputTexture, Scene.CurrentCamera, time);
    }

    public void Render(CommandBuffer commandBuffer, Texture output, CameraComponent camera, GameTime time)
    {
        UpdateFrame(time);

        RenderPassColorAttachment colorAttachment = new(output.DefaultView!, Colors.Black)
        {
            LoadAction = LoadAction.Clear,
            StoreAction = MultisampleColorTexture != null ? StoreAction.Discard : StoreAction.Store,
            ClearColor = new Color(0.3f, 0.3f, 0.3f)
        };
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor renderPassDescriptor = new(depthStencilAttachment, colorAttachment)
        {
            Label = "Forward pass"u8
        };

        RenderPassEncoder renderPass = commandBuffer.BeginRenderPass(renderPassDescriptor);

        // Frame BindGroup (once per frame)
        renderPass.SetBindGroup(FrameBindGroupSpace, _frameBindGroup);

        // For each camera
        RenderCamera(renderPass, camera);

        renderPass.EndEncoding();

        // Clear the render batch. It'll be built up again next frame.
        _renderBatch.Clear();
    }

    private void RenderCamera(RenderPassEncoder passEncoder, CameraComponent camera)
    {
        UpdateCamera(camera);

        passEncoder.SetBindGroup(ViewBindGroupSpace, _viewBindGroup);
        BindGroup instanceBindGroup = _renderBatch.UpdateInstanceBuffer(Device.FrameIndex);
        passEncoder.SetBindGroup(InstanceBindGroupSpace, instanceBindGroup);

        // Loop through all the renderable entities and store them by pipeline.
        foreach (var pipeline in _renderBatch.SortedPipelines)
        {
            passEncoder.SetPipeline(pipeline.Pipeline);

            var geometryList = _renderBatch.GetGeometryList(pipeline);
            foreach (var pair in geometryList)
            {
                SubMesh geometry = pair.Key;
                var materialList = pair.Value;

                passEncoder.SetVertexBuffer(0, geometry.Mesh.GpuVertexBuffer!);
                passEncoder.SetIndexBuffer(geometry.Mesh.GpuIndexBuffer!, geometry.Mesh.IndexFormat);

                foreach (var materialPair in materialList)
                {
                    var material = materialPair.Key;
                    var instances = materialPair.Value;

                    // TODO: GPUBindGroups
                    //int i = material.firstBindGroupIndex;
                    //for (const bindGroup of material.bindGroups) {
                    //    passEncoder.setBindGroup(i++, bindGroup);
                    //}

                    passEncoder.SetBindGroup(0, material);

                    // Update per-object data (after set pipeline)
                    //InstanceData drawData = new()
                    //{
                    //    WorldMatrix = instances.Transforms[0]
                    //};
                    //passEncoder.SetPushConstants(0, drawData);

                    uint instanceCount = instances.InstanceCount + instances.FirstInstance;
                    passEncoder.DrawIndexed((uint)geometry.IndexCount, instanceCount, (uint)geometry.IndexStart, 0, instances.FirstInstance);
                }
            }
        }
    }

    public void Resize(int pixelWidth, int pixelHeight)
    {
        Width = pixelWidth * ResolutionMultiplier;
        Height = pixelHeight * ResolutionMultiplier;

        if (SampleCount > TextureSampleCount.Count1)
        {
            MultisampleColorTexture?.Dispose();

            TextureDescriptor multisampleTextureDesc = TextureDescriptor.Texture2D(ColorFormat, (uint)Width, (uint)Height, 1, 1, TextureUsage.RenderTarget, SampleCount);
            MultisampleColorTexture = Device.CreateTexture(in multisampleTextureDesc);
        }

        if (DepthStencilFormat != PixelFormat.Undefined &&
            DepthStencilFormat.IsDepthStencilFormat())
        {
            DepthStencilTexture?.Dispose();

            TextureDescriptor depthStencilTextureDesc = TextureDescriptor.Texture2D(DepthStencilFormat, (uint)Width, (uint)Height, 1, 1,
                usage: TextureUsage.RenderTarget,
                sampleCount: SampleCount
                );
            DepthStencilTexture = Device.CreateTexture(in depthStencilTextureDesc);
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
            projectionMatrix = camera.ProjectionMatrix,
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
        return Device.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 1, 1);
    }

    protected override void OnEntityComponentAdded(MeshComponent component)
    {
        foreach (Material material in component.Materials)
        {
            if (!_gpuMaterialFactories.TryGetValue(material.GetType(), out IGPUMaterialFactory? factory))
            {
                factory = GPUMaterialFactories.GetFactory(material, this);
                _gpuMaterialFactories[material.GetType()] = factory;
            }

            factory.Add(material);
        }
    }

    protected override void OnEntityComponentRemoved(MeshComponent component)
    {
        foreach (Material material in component.Materials)
        {
            if (_gpuMaterialFactories.TryGetValue(material.GetType(), out IGPUMaterialFactory? factory))
            {
                factory.Remove(material);
            }
        }
    }

    class RenderPrimitive
    {
        public readonly SubMesh Geometry;
        public readonly GPURenderPipeline Pipeline;
        public readonly GPUMaterialBindGroups BindGroups;

        public RenderPrimitive(SubMesh geometry, GPURenderPipeline pipeline, BindGroup bindGroup)
        {
            Geometry = geometry;
            Pipeline = pipeline;
            BindGroups = new GPUMaterialBindGroups(bindGroup);
        }

        public RenderPrimitive(SubMesh geometry, GPURenderPipeline pipeline, GPUMaterialBindGroups? bindGroups)
        {
            Geometry = geometry;
            Pipeline = pipeline;
            BindGroups = bindGroups ?? new GPUMaterialBindGroups();
        }
    }
}
