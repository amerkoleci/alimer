// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using static Alimer.Graphics.Constants;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Rendering;

public sealed class PhysicallyBasedMaterialFactory : GPUMaterialFactory<PhysicallyBasedMaterial>
{
    private const uint InitialCount = 64;
    private static uint GpuStructureSizeInBytes = SizeOf<GPUMaterialPBR>();

    private uint _materialCapacity = 0;
    private readonly GpuBuffer[] _materialBuffer;
    private unsafe GPUMaterialPBR* _mappedMaterialData;
    private int _bindlessShaderReadIndex = InvalidBindlessIndex;
    private int _materialCount = 0;

    public PhysicallyBasedMaterialFactory(RenderSystem system)
        : base(system)
    {
        _materialBuffer = new GpuBuffer[system.Device.MaxFramesInFlight];
        ResizeBuffer(InitialCount);
    }

    private void ResizeBuffer(uint capacity)
    {
        _materialCapacity = capacity;

        for (int i = 0; i < _materialBuffer.Length; i++)
        {
            _materialBuffer[i]?.Dispose();

            BufferDescriptor descriptor = new(GpuStructureSizeInBytes * _materialCapacity, BufferUsage.ShaderRead, MemoryType.Upload, label: $"PBR Material Buffer {i}");
            _materialBuffer[i] = ToDispose(System.Device.CreateBuffer(in descriptor));
        }
    }

    public override unsafe void BeginFrame(uint frameIndex)
    {
        _materialCount = 0;
        _mappedMaterialData = (GPUMaterialPBR*)_materialBuffer[frameIndex].GetMappedData();
        _bindlessShaderReadIndex = _materialBuffer[frameIndex].BindlessShaderReadIndex;
    }

    protected override unsafe int Write(PhysicallyBasedMaterial material, out int materialIndex)
    {
        Texture baseColorTexture = material.BaseColorTexture ?? System.OpaqueWhiteTexture;
        Texture normalTexture = material.NormalTexture ?? System.DefaultNormalTexture;
        Texture metallicRoughnessTexture = material.MetallicRoughnessTexture ?? System.OpaqueWhiteTexture;
        Texture emissiveTexture = material.EmissiveTexture ?? System.OpaqueWhiteTexture;
        Texture occlusionTexture = material.OcclusionTexture ?? System.OpaqueWhiteTexture;

        _mappedMaterialData[_materialCount] = new()
        {
            BaseIndex = baseColorTexture.DefaultView!.BindlessShaderReadIndex,
            NormalIndex = normalTexture.DefaultView!.BindlessShaderReadIndex,
            MetallicRoughnessIndex = metallicRoughnessTexture.DefaultView!.BindlessShaderReadIndex,
            EmissiveIndex = emissiveTexture.DefaultView!.BindlessShaderReadIndex,
            OcclusionIndex = occlusionTexture.DefaultView!.BindlessShaderReadIndex,
            SamplerIndex = System.DefaultSampler.BindlessIndex,
            baseColorFactor = material.BaseColorFactor,
            emissiveFactor = material.EmissiveFactor,
            normalScale = material.NormalScale,
            metallicRoughnessFactor = new(material.MetallicFactor, material.RoughnessFactor),
            occlusionStrength = material.OcclusionStrength,
            alphaCutoff = material.AlphaCutoff,
            baseColorUVSet = (int)material.BaseColorUVChannel,
            normalUVSet = (int)material.NormalTextureUVChannel,
            emissiveUVSet = (int)material.EmissiveTextureUVChannel,
            metallicRoughnessUVSet = (int)material.MetallicRoughnessTextureUVChannel,
        };

        materialIndex = _materialCount++;
        return _bindlessShaderReadIndex;
    }


    protected override PipelineLayout CreatePipelineLayout(bool skinned)
    {
        Span<BindGroupLayout> bindGroupLayouts = [
            System.EmptyBindGroupLayout,
            System.ViewBindGroupLayout,
            System.FrameBindGroupLayout];

        PipelineLayoutDescriptor descriptor = new(bindGroupLayouts);
        return System.Device.CreatePipelineLayout(in descriptor);
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

    private static bool IsFullyRough(PhysicallyBasedMaterial material)
    {
        return material.RoughnessFactor == 1.0 && material.MetallicRoughnessTexture is null;
    }
}
