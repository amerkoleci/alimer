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
    private readonly GPUBuffer[] _materialBuffer;
    private readonly GPUBufferView[] _materialBufferView;
    private unsafe GPUMaterialPBR* _mappedMaterialData;
    private int _bindlessShaderReadIndex = InvalidBindlessIndex;
    private int _materialCount = 0;

    public PhysicallyBasedMaterialFactory(RenderSystem system)
        : base(system)
    {
        _materialBuffer = new GPUBuffer[system.Device.MaxFramesInFlight];
        _materialBufferView = new GPUBufferView[system.Device.MaxFramesInFlight];
        ResizeBuffer(InitialCount);
    }

    private void ResizeBuffer(uint capacity)
    {
        _materialCapacity = capacity;
        GPUBufferViewDescriptor viewDescriptor = GPUBufferViewDescriptor.CreateStructured(0, _materialCapacity, GpuStructureSizeInBytes);

        for (int i = 0; i < _materialBuffer.Length; i++)
        {
            _materialBuffer[i]?.Dispose();

            GPUBufferDescriptor descriptor = new(GpuStructureSizeInBytes * _materialCapacity, GPUBufferUsage.ShaderRead, MemoryType.Upload, label: $"PBR Material Buffer {i}");
            _materialBuffer[i] = ToDispose(System.Device.CreateBuffer(in descriptor));
            _materialBufferView[i] = ToDispose(_materialBuffer[i].CreateView(viewDescriptor));
        }
    }

    public override unsafe void BeginFrame(uint frameIndex)
    {
        _materialCount = 0;
        _mappedMaterialData = (GPUMaterialPBR*)_materialBuffer[frameIndex].GetMappedData();
        _bindlessShaderReadIndex = _materialBufferView[frameIndex].BindlessReadIndex;
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
            BaseIndex = baseColorTexture.DefaultView!.BindlessReadIndex,
            NormalIndex = normalTexture.DefaultView!.BindlessReadIndex,
            MetallicRoughnessIndex = metallicRoughnessTexture.DefaultView!.BindlessReadIndex,
            EmissiveIndex = emissiveTexture.DefaultView!.BindlessReadIndex,
            OcclusionIndex = occlusionTexture.DefaultView!.BindlessReadIndex,
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
