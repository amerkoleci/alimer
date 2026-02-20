// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed class PhysicallyBasedMaterialFactory : GPUMaterialFactory<PhysicallyBasedMaterial>
{
    private readonly GraphicsBuffer _materialBuffer;

    public unsafe PhysicallyBasedMaterialFactory(RenderSystem system)
        : base(system)
    {
        BufferDescriptor bufferDescriptor = new((uint)sizeof(PBRMaterialData), BufferUsage.Constant, MemoryType.Upload, label: "PBR Material Buffer");
        _materialBuffer = ToDispose(system.Device.CreateBuffer(in bufferDescriptor));
    }

    protected override BindGroupLayout CreateBindGroupLayout()
    {
        return System.Device.CreateBindGroupLayout(
            "PBR Material BindGroupLayout",
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Fragment),
            new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment), // baseColorTexture
            new BindGroupLayoutEntry(new SamplerBindingLayout(SamplerBindingType.Filtering), 0),  // baseColorSampler
            new BindGroupLayoutEntry(new TextureBindingLayout(), 1, ShaderStages.Fragment), // normalTexture
            new BindGroupLayoutEntry(new TextureBindingLayout(), 2, ShaderStages.Fragment), // metallicRoughnessTexture 
            new BindGroupLayoutEntry(new TextureBindingLayout(), 3, ShaderStages.Fragment), // emissiveTexture 
            new BindGroupLayoutEntry(new TextureBindingLayout(), 4, ShaderStages.Fragment), // occlusionTexture 

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
            new BindGroupLayoutEntry(SamplerDescriptor.ComparisonDepth, 109, ShaderStages.All)      // SamplerAnisotropicMirror
            );
    }

    public override ShaderModule CreateFragmentShaderModule(Span<VertexBufferLayout> geometryLayout, Material material)
    {
        if (material is not PhysicallyBasedMaterial pbrMaterial)
            throw new ArgumentException("Material must be of type PhysicallyBasedMaterial", nameof(material));

        // Optimized version of PBR?
        bool roughnessOnly = IsFullyRough(pbrMaterial);
        ShaderModule fragmentShader = System.ShaderSystem.GetShaderModule("PBR", ShaderStages.Fragment);
        return fragmentShader;
    }

    protected override BindGroup CreateBindGroup(Material material)
    {
        if (material is not PhysicallyBasedMaterial pbrMaterial)
            throw new ArgumentException("Material must be of type PhysicallyBasedMaterial", nameof(material));

        // TODO: Should be one material buffer per material
        PBRMaterialData materialData = new()
        {
            baseColorFactor = pbrMaterial.BaseColorFactor,
            emissiveFactor = pbrMaterial.EmissiveFactor,
            metallicFactor = pbrMaterial.MetallicFactor,
            roughnessFactor = pbrMaterial.RoughnessFactor,
            normalScale = pbrMaterial.NormalScale,
            occlusionStrength = pbrMaterial.OcclusionStrength
        };
        _materialBuffer.SetData(materialData);

        BindGroup bindGroup = _bindGroupLayout.CreateBindGroup(
            new BindGroupEntry(0, _materialBuffer),
            new BindGroupEntry(0, pbrMaterial.BaseColorTexture ?? System.OpaqueWhiteTexture),
            new BindGroupEntry(1, pbrMaterial.NormalTexture ?? System.DefaultNormalTexture),
            new BindGroupEntry(2, pbrMaterial.MetallicRoughnessTexture ?? System.OpaqueWhiteTexture),
            new BindGroupEntry(3, pbrMaterial.EmissiveTexture ?? System.OpaqueWhiteTexture),
            new BindGroupEntry(4, pbrMaterial.OcclusionTexture ?? System.OpaqueWhiteTexture),
            new BindGroupEntry(0, System.DefaultSampler)
        );

        return bindGroup;
    }

    private static bool IsFullyRough(PhysicallyBasedMaterial material)
    {
        return material.RoughnessFactor == 1.0 && material.MetallicRoughnessTexture is null;
    }
}
