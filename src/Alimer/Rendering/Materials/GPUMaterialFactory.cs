// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using static Alimer.Rendering.RenderSystem;

namespace Alimer.Rendering;

public interface IGPUMaterialFactory : IDisposable
{
    void Add(Material material);
    bool Remove(Material material);

    //VertexBufferLayout GetLayout(Mesh mesh);
    RenderPipeline GetPipeline(Span<VertexBufferLayout> geometryLayout, Material material, bool skinned);
    BindGroup GetBindGroup(Material material, bool skinned);
}

public abstract class GPUMaterialFactory<TMaterial> : DisposableObject, IGPUMaterialFactory
    where TMaterial : Material
{
    protected readonly List<TMaterial> _materials = [];
    protected BindGroupLayout _bindGroupLayout;
    private readonly Dictionary<int, BindGroup> _bindGroups = [];
    private readonly Dictionary<int, RenderPipeline> _renderPipelines = [];

    protected GPUMaterialFactory(RenderSystem system)
    {
        Guard.IsNotNull(system, nameof(system));

        System = system;
        _bindGroupLayout = ToDispose(CreateBindGroupLayout());
    }

    public RenderSystem System { get; }

    #region IGPUMaterialFactory Members
    void IGPUMaterialFactory.Add(Material material)
    {
        if (material is not TMaterial typedMaterial)
            throw new ArgumentException($"Material must be of type {typeof(TMaterial).FullName}", nameof(material));

        _materials.Add(typedMaterial);
    }


    bool IGPUMaterialFactory.Remove(Material material)
    {
        if (material is not TMaterial typedMaterial)
            throw new ArgumentException($"Material must be of type {typeof(TMaterial).FullName}", nameof(material));

        return _materials.Remove(typedMaterial);
    }
    #endregion

    protected abstract BindGroupLayout CreateBindGroupLayout();
    protected abstract BindGroup CreateBindGroup(Material material);

    public virtual ShaderModule CreateVertexShaderModule(Span<VertexBufferLayout> geometryLayout, Material material, bool skinned)
    {
        return System.ShaderSystem.GetShaderModule("VertexCommon", ShaderStages.Vertex);
    }

    public abstract ShaderModule CreateFragmentShaderModule(Span<VertexBufferLayout> geometryLayout, Material material);

    protected virtual unsafe PipelineLayout CreatePipelineLayout(bool skinned)
    {
        Span<BindGroupLayout> bindGroupLayouts = [_bindGroupLayout, System.ViewBindGroupLayout, System.FrameBindGroupLayout];
        Span<PushConstantRange> pushConstantRanges = [
            new PushConstantRange(999, (uint)sizeof(DrawData))
        ];

        PipelineLayoutDescriptor descriptor = new(bindGroupLayouts, pushConstantRanges);
        return System.Device.CreatePipelineLayout(in descriptor);
    }

    public BindGroup GetBindGroup(Material material, bool skinned)
    {
        // TODO: Handle skin
        int hashCode = HashCode.Combine(material.Id.GetHashCode(), skinned);
        if (!_bindGroups.TryGetValue(hashCode, out BindGroup? bindGroup))
        {
            bindGroup = CreateBindGroup(material);
            _bindGroups[hashCode] = ToDispose(bindGroup);
        }

        // TODO: Handle bind group update

        return bindGroup;
    }

    public RenderPipeline GetPipeline(Span<VertexBufferLayout> geometryLayout, Material material, bool skinned)
    {
        int hashCode = GetPipelineKey(geometryLayout, material, skinned);
        if (!_renderPipelines.TryGetValue(hashCode, out RenderPipeline? pipeline))
        {
            ShaderModule vertexShader = CreateVertexShaderModule(geometryLayout, material, skinned);
            ShaderModule fragmentShader = CreateFragmentShaderModule(geometryLayout, material);
            PipelineLayout pipelineLayout = ToDispose(CreatePipelineLayout(skinned));

            // TODO: Handle BlendState, RasterizerState, DepthStencilState, PrimitiveTopology, etc.

            RenderPipelineDescriptor renderPipelineDesc = new(pipelineLayout,
                geometryLayout,
                [System.ColorFormat], System.DepthStencilFormat)
            {
                Label = "RenderPipeline",
                VertexShader = vertexShader,
                FragmentShader = fragmentShader
            };
            pipeline = System.Device.CreateRenderPipeline(in renderPipelineDesc);

            _renderPipelines[hashCode] = ToDispose(pipeline);
        }

        return pipeline;
    }

    protected virtual int GetPipelineKey(Span<VertexBufferLayout> geometryLayout, Material material, bool skinned)
    {
        int hashCode = 0;
        foreach (VertexBufferLayout layout in geometryLayout)
        {
            hashCode = HashCode.Combine(hashCode, layout.GetHashCode());
        }

        hashCode = HashCode.Combine(hashCode, material.Id.GetHashCode(), skinned);
        return hashCode;
    }
}

public static class GPUMaterialFactories
{
    private static readonly Dictionary<Type, Func<RenderSystem, IGPUMaterialFactory>> s_factories = [];

    public static void RegisterDefault()
    {
        // Register built-in material factories here
        Register<UnlitMaterial>((system) => new UnlitMaterialFactory(system));
        Register<PhysicallyBasedMaterial>((system) => new PhysicallyBasedMaterialFactory(system));
    }

    public static void Register<TMaterial>(Func<RenderSystem, IGPUMaterialFactory> factory)
        where TMaterial : Material
    {
        s_factories[typeof(TMaterial)] = factory;
    }

    public static IGPUMaterialFactory GetFactory(Material material, RenderSystem system)
    {
        Type materialType = material.GetType();
        if (!s_factories.TryGetValue(materialType, out Func<RenderSystem, IGPUMaterialFactory>? factoryFunc))
        {
            throw new InvalidOperationException($"No GPU material factory registered for material type '{materialType.FullName}'.");
        }

        return factoryFunc!(system);
    }
}
