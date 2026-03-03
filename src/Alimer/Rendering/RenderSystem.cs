// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Rendering;

// TODO: Add support for RenderableComponent (ParticleSystem or other?)

public sealed partial class RenderSystem : EntitySystem<MeshComponent>
{
    // ShaderTypes.h
    public const int FrameBindGroupSpace = 3;
    public const int ViewBindGroupSpace = 2;
    public const int InstanceBindGroupSpace = 1;
    public const int MaterialBindGroupSpace = 0;

    private readonly Dictionary<Type, IGPUMaterialFactory> _gpuMaterialFactories = [];
    private readonly RenderBatch _renderBatch;

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

        OpaqueWhiteTexture = CreateTextureFromColor(Colors.White);
        TransparentBlackTexture = CreateTextureFromColor(Colors.Transparent);
        DefaultNormalTexture = CreateTextureFromColor(new Color(0.5f, 0.5f, 1.0f, 0f));

        // A simple 1x1 black environment map
        var environmentTextureDesc = TextureDescriptor.TextureCube(PixelFormat.RG11B10Float, 1, label: "Default Environment Map");
        DefaultEnvironmentTexture = ToDispose(Device.CreateTexture(in environmentTextureDesc));

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

        // TODO: Implement ring buffer

        // Per frame (set 3)
        {
            FrameBindGroupLayout = ToDispose(Device.CreateBindGroupLayout(
                "Frame BindGroupLayout",
                new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.All),
                new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment), // environmentTexture
                new BindGroupLayoutEntry(SamplerDescriptor.LinearWrap, 0)  // environmentSampler
                ));

            // TODO: Configure environment texture/sampler in frame bind group
            string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
            EnvironmentMap = ToDispose(Texture.FromFile(Device, Path.Combine(texturesPath, "zavelstein_ibl.ktx")));

            FrameConstantBuffer = ToDispose(new ConstantBuffer<FrameConstants>(Device, label: "Frame Constant Buffer"));
            FrameBindGroup = ToDispose(FrameBindGroupLayout.CreateBindGroup(
                "Frame BindGroup",
                new BindGroupEntry(0, FrameConstantBuffer.Handle),
                new BindGroupEntry(0, EnvironmentMap)
                ));
        }


        // Per view (set 2)
        {
            uint hackBinding = 1; // Should be 0 
            ViewBindGroupLayout = ToDispose(Device.CreateBindGroupLayout(
                "View BindGroupLayout",
                new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.All),
                new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.ShaderRead), hackBinding, ShaderStages.Fragment)
            ));

            ViewConstantBuffer = ToDispose(new ConstantBuffer<PerViewData>(Device, label: "View Constant Buffer"));

            // Light structured buffer 
            BufferDescriptor descriptor = new(SizeOf<LightData>() * 6, BufferUsage.ShaderRead, MemoryType.Upload, "Lights Structured Buffer");
            LightsStructuredBuffer = ToDispose(Device.CreateBuffer(in descriptor));

            ViewBindGroup = ToDispose(ViewBindGroupLayout.CreateBindGroup(
                "View BindGroup",
                new BindGroupEntry(0, ViewConstantBuffer.Handle),
                new BindGroupEntry(hackBinding, LightsStructuredBuffer)
                ));
        }

        // Skybox renderer
        SkyboxRenderer = ToDispose(new SkyboxRenderer(this));

        MainWindow.SizeChanged += OnCanvasSizeChanged;
        Resize(MainWindow.SizeInPixels);
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
    public Texture? EnvironmentMap { get; set; }

    public Texture OpaqueWhiteTexture { get; }
    public Texture TransparentBlackTexture { get; }
    public Texture DefaultNormalTexture { get; }
    public Texture DefaultEnvironmentTexture { get; }
    public Texture CheckerTexture { get; }
    public Sampler DefaultSampler { get; }

    public BindGroupLayout InstanceBindGroupLayout => _renderBatch.InstanceBindGroupLayout; // 1

    // Set 2 (per view/camera)
    public BindGroupLayout ViewBindGroupLayout { get; } // 2
    public ConstantBuffer<PerViewData> ViewConstantBuffer { get; } // b0
    public GraphicsBuffer LightsStructuredBuffer { get; } // b1 // LightData
    public BindGroup ViewBindGroup { get; }

    // Set 3 (per frame)
    public BindGroupLayout FrameBindGroupLayout { get; } // 3
    public ConstantBuffer<FrameConstants> FrameConstantBuffer { get; }
    public BindGroup FrameBindGroup { get; }
    public SkyboxRenderer SkyboxRenderer { get; }
    public ShaderSystem ShaderSystem { get; }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        base.Destroy();

        // Dispose all material factories
        foreach (IGPUMaterialFactory factory in _gpuMaterialFactories.Values)
        {
            factory.Dispose();
        }
        _gpuMaterialFactories.Clear();

        MultisampleColorTexture?.Dispose();
        DepthStencilTexture?.Dispose();
    }

    public override void Update(GameTime time)
    {
        if (Scene.CurrentCamera is null)
            return;

        // Clear the render batch. It'll be built up again next frame.
        _renderBatch.Clear();

        // Prepare frame instances (visible to camera)
        BoundingFrustum cameraFrustum = Scene.CurrentCamera.Frustum;

        foreach (MeshComponent meshComponent in Components)
        {
            // Skip rendering if no mesh is assigned
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
        UpdateLights();

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
        renderPass.SetBindGroup(FrameBindGroupSpace, FrameBindGroup);

        // For each camera
        RenderCamera(renderPass, camera);

        // Draw skybox from environment map
        if (EnvironmentMap is not null)
        {
            SkyboxRenderer.Draw(renderPass);
        }

        renderPass.EndEncoding();
    }

    private void RenderCamera(RenderPassEncoder passEncoder, CameraComponent camera)
    {
        UpdateCamera(camera);

        // Set 2 (per view/camera)
        passEncoder.SetBindGroup(ViewBindGroupSpace, ViewBindGroup);

        // Set 1 (per instance data)
        BindGroup instanceBindGroup = _renderBatch.UpdateInstanceBuffer(Device.FrameIndex);
        passEncoder.SetBindGroup(InstanceBindGroupSpace, instanceBindGroup);

        // Loop through all the renderable entities and store them by pipeline.
        foreach (GPURenderPipeline pipeline in _renderBatch.SortedPipelines)
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
                    //    worldMatrix = instances.Transforms[0]
                    //};
                    //passEncoder.SetPushConstants(0, drawData);

                    uint instanceCount = instances.InstanceCount + instances.FirstInstance;
                    passEncoder.DrawIndexed((uint)geometry.IndexCount, instanceCount, (uint)geometry.IndexStart, 0, instances.FirstInstance);
                }
            }
        }
    }

    public void Resize(in SizeI sizeInPixels)
    {
        Width = sizeInPixels.Width * ResolutionMultiplier;
        Height = sizeInPixels.Height * ResolutionMultiplier;

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
        Resize(MainWindow.SizeInPixels);
    }

    private void UpdateFrame(GameTime gameTime)
    {
        FrameConstants frameData = new()
        {
            ElapsedTime = (float)gameTime.Elapsed.TotalSeconds,
            TotalTime = (float)gameTime.Total.TotalSeconds
        };

        FrameConstantBuffer.SetData(frameData);
    }

    private void UpdateCamera(CameraComponent camera)
    {
        LightSystem? lightSystem = EntityManager?.Systems.Get<LightSystem>();

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
        viewData.ambientLight = Vector3.One;
        viewData.activeLightCount = 0;
        if (lightSystem is not null)
        {
            viewData.activeLightCount = (uint)lightSystem.Lights.Count;
        }

        ViewConstantBuffer.SetData(viewData);
    }

    private void UpdateLights()
    {
        LightSystem? lightSystem = EntityManager?.Systems.Get<LightSystem>();

        if (lightSystem is null)
            return;

        // Sort lights as well
        int lightCount = lightSystem.Lights.Count;
        Span<LightData> lightData = new LightData[lightSystem.Lights.Count];
        int lightIndex = 0;

        foreach (LightComponent light in lightSystem.Lights)
        {
            lightData[lightIndex] = new LightData
            {
                position = light.Entity!.WorldTransform.Translation,
                direction = light.Entity!.Direction,
                color = light.Color.ToVector3(),
                intensity = light.Intensity,
                type = (light.LightType == LightType.Directional) ? ShaderLightType.Directional : ShaderLightType.Point
            };
            lightIndex++;
        }


        LightsStructuredBuffer.SetData(lightData);
        //DirectionalLightGroupBuffer.SetData(lightData.AsSpan(), Unsafe.SizeOf<Vector4>());
    }

    private Texture CreateTextureFromColor(in Color color)
    {
        ReadOnlySpan<uint> pixels = [color.ToRgba()];
        return ToDispose(Device.CreateTexture2D(pixels, PixelFormat.RGBA8Unorm, 1, 1));
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
