// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public struct GraphicsDeviceLimits
{
    public uint MaxTextureDimension1D;
    public uint MaxTextureDimension2D;
    public uint MaxTextureDimension3D;
    public uint MaxTextureDimensionCube;
    public uint MaxTextureArrayLayers;
    public uint MaxTexelBufferDimension2D;

    public uint UploadBufferTextureRowAlignment;
    public uint UploadBufferTextureSliceAlignment;
    public uint ConstantBufferMinOffsetAlignment;
    public uint ConstantBufferMaxRange;

    public uint StorageBufferMinOffsetAlignment;
    public uint StorageBufferMaxRange;

    public ulong MaxBufferSize;
    public uint MaxPushConstantsSize;

    public uint MaxVertexBuffers;
    public uint MaxVertexAttributes;
    public uint MaxVertexBufferArrayStride;

    public uint MaxViewports;
    public uint MaxColorAttachments;

    public uint MaxComputeWorkgroupStorageSize;
    public uint MaxComputeInvocationsPerWorkGroup;
    public uint MaxComputeWorkGroupSizeX;
    public uint MaxComputeWorkGroupSizeY;
    public uint MaxComputeWorkGroupSizeZ;
    public uint MaxComputeWorkGroupsPerDimension;

    public ushort SamplerMaxAnisotropy;
    public float SamplerMinLodBias;
    public float SamplerMaxLodBias;

    // VariableRateShading
    public uint VariableRateShadingTileSize;

    // Ray tracing
    public ulong RayTracingShaderGroupIdentifierSize;
    public ulong RayTracingShaderTableAligment;
    public ulong RayTracingShaderTableMaxStride;
    public uint RayTracingShaderRecursionMaxDepth;
    public uint RayTracingMaxGeometryCount;
}
