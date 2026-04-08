// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;
using static Alimer.Utilities.UnsafeUtilities;
using static Alimer.Graphics.Constants;

namespace Alimer.Rendering;

// TODO: Add support for RenderableComponent (ParticleSystem or other?)

public sealed partial class RenderSystem : EntitySystem<MeshComponent>
{
    // ShaderTypes.h
    private const uint MinLightCapacity = 32;
    private static uint GpuLightStructureSizeInBytes = SizeOf<GPULight>();

    private readonly Dictionary<Type, IGPUMaterialFactory> _gpuMaterialFactories = [];
    private readonly RenderBatch _renderBatch;

    private readonly UnlitMaterial _defaultMaterial = new();

    private uint _lightCapacity;
    private GpuBuffer? _lightBuffer;
    private GpuBufferView? _lightBufferView;

    public unsafe RenderSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        Debug.Assert(GpuLightStructureSizeInBytes == 64, "GPULight must be 64 bytes");
        Debug.Assert(sizeof(GPUInstance) == 80, "GPUInstance must be 80 bytes");

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
        FrameBindGroupLayout = ToDispose(Device.CreateBindGroupLayout(
            "Frame BindGroupLayout",
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.All)
            ));

        // TODO: Configure environment texture/sampler in frame bind group
        string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        EnvironmentMap = ToDispose(Texture.FromFile(Device, Path.Combine(texturesPath, "zavelstein_ibl.ktx")));

        FrameConstantBuffer = ToDispose(new ConstantBuffer<GPUFrameData>(Device, label: "Frame Constant Buffer"));

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

    public BindGroupLayout FrameBindGroupLayout { get; } // 3
    public ConstantBuffer<GPUFrameData> FrameConstantBuffer { get; }
    //public BindGroup FrameBindGroup { get; }
    public SkyboxRenderer SkyboxRenderer { get; }
    public ShaderSystem ShaderSystem { get; }

    /// <summary>Finalizes an instance of the <see cref="RenderSystem" /> class.</summary>
    ~RenderSystem() => Dispose(disposing: false);

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        base.Dispose(disposing);

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

            _lightBufferView?.Dispose();
            _lightBuffer?.Dispose();
        }
    }

    public override void Update(GameTime time)
    {
        if (Scene.CurrentCamera is null)
            return;

        // Clear the render batch. It'll be built up again next frame.
        _renderBatch.Clear();

        // Prepare frame instances (visible to camera)
        BoundingFrustum cameraFrustum = Scene.CurrentCamera.Frustum;

        foreach (var factory in _gpuMaterialFactories.Values)
        {
            factory.BeginFrame(Device.FrameIndex);
        }

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

                int bindlessShaderReadIndex = factory.Write(material, out int gpuMaterialIndex);

                Matrix4x4 worldMatrix = meshComponent.Entity!.WorldTransform;
                GPUInstanceData instanceData = new(worldMatrix, gpuMaterialIndex);

                bool skinned = false;
                GPURenderPipeline pipeline = factory.GetPipeline(geometryLayout, material, skinned);
                //RenderPrimitive primitive = new(subMesh, pipeline, bindGroups);
                _renderBatch.AddRenderable(subMesh, pipeline, bindlessShaderReadIndex, instanceData);
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

        // For each camera
        RenderCamera(renderPass, camera, time);

        // Draw skybox from environment map
        if (EnvironmentMap is not null)
        {
            SkyboxRenderer.Draw(renderPass);
        }

        renderPass.EndEncoding();
    }

    private void RenderCamera(RenderPassEncoder passEncoder, CameraComponent camera, GameTime time)
    {
        UpdateFrame(camera, time);

        // Per frame + camera/view
        passEncoder.SetConstantBuffer(0, FrameConstantBuffer.Handle);

        // Set 1 (per instance data)
        _renderBatch.UpdateInstanceBuffer(Device.FrameIndex);

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
                    int materialBufferIndex = materialPair.Key;
                    GPUBatchEntry instances = materialPair.Value;

                    PBRPushConstants pushConstants = new()
                    {
                        InstanceBufferIndex = instances.InstanceBufferIndex,
                        MaterialBufferIndex = materialBufferIndex,
                        LightBufferIndex = _lightBufferView!.BindlessReadIndex
                    };
                    passEncoder.SetPushConstants(pushConstants);

                    uint instanceCount = instances.InstanceCount + instances.FirstInstance;
                    passEncoder.DrawIndexed((uint)geometry.IndexCount,
                        instanceCount,
                        (uint)geometry.IndexStart,
                        0,
                        instances.FirstInstance
                        );
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

    private void UpdateFrame(CameraComponent camera, GameTime gameTime)
    {
        LightSystem? lightSystem = EntityManager?.Systems.Get<LightSystem>();

        GPUFrameData frameData = new()
        {
            viewMatrix = camera.ViewMatrix,
            projectionMatrix = camera.ProjectionMatrix,
            viewProjectionMatrix = camera.ViewProjectionMatrix
        };
        _ = Matrix4x4.Invert(frameData.viewProjectionMatrix, out frameData.inverseViewMatrix);
        _ = Matrix4x4.Invert(frameData.projectionMatrix, out frameData.inverseProjectionMatrix);

        // https://github.com/Aminator/DirectX12GameEngine/blob/master/DirectX12GameEngine.Rendering/Materials/MaterialAttributes.cs
        frameData.cameraPosition = camera.Entity!.Transform.Position;
        frameData.ambientLight = new Vector3(0.05f, 0.05f, 0.05f);
        frameData.activeLightCount = 0;
        if (lightSystem is not null)
        {
            frameData.activeLightCount = (uint)lightSystem.Lights.Count;
        }
        frameData.EnvironmentTextureIndex = EnvironmentMap is not null ? EnvironmentMap.DefaultView!.BindlessReadIndex : InvalidBindlessIndex;
        frameData.elapsedTime = (float)gameTime.Elapsed.TotalSeconds;
        frameData.totalTime = (float)gameTime.Total.TotalSeconds;

        FrameConstantBuffer.SetData(frameData);
    }

    private unsafe void UpdateLights()
    {
        LightSystem? lightSystem = EntityManager?.Systems.Get<LightSystem>();

        if (lightSystem is null)
            return;

        // Sort lights as well
        uint lightCount = (uint)lightSystem.Lights.Count;
        if (_lightBuffer is null ||
            lightCount > _lightCapacity)
        {
            _lightBuffer?.Dispose();

            _lightCapacity = Math.Max(MinLightCapacity, lightCount);
            GpuBufferDescriptor descriptor = new(GpuLightStructureSizeInBytes * _lightCapacity, GpuBufferUsage.ShaderRead, MemoryType.Upload, label: "Lights Structured Buffer");
            GpuBufferViewDescriptor viewDescriptor = GpuBufferViewDescriptor.CreateStructured(0, _lightCapacity, GpuLightStructureSizeInBytes);

            _lightBuffer = Device.CreateBuffer(in descriptor);
            _lightBufferView = _lightBuffer.CreateView(viewDescriptor);
        }

        // TODO: Resize Light Buffer if light count exceeds current capacity
        GPULight* lightData = (GPULight*)_lightBuffer.GetMappedData();

        int lightIndex = 0;

        // Write light data directly into the persistently-mapped buffer
        foreach (LightComponent light in lightSystem.Lights)
        {
            lightData[lightIndex] = new GPULight
            {
                Position = light.Entity!.WorldTransform.Translation,
                Direction = light.Entity!.Direction,
                Color = light.Color.ToVector3() * light.Intensity,
                //intensity = light.Intensity,
                Type = (light.LightType == LightType.Directional) ? ShaderLightType.Directional : ShaderLightType.Point
            };
            lightIndex++;
        }
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
}
