// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public struct GraphicsDeviceLimits
{
    public uint MaxTextureDimension1D;
    public uint MaxTextureDimension2D;
    public uint MaxTextureDimension3D;
    public uint MaxTextureDimensionCube;
    public uint MaxTextureArrayLayers;
    public uint MaxBindGroups;
    //public uint MaxTexelBufferDimension2D;

    //public uint UploadBufferTextureRowAlignment;
    //public uint UploadBufferTextureSliceAlignment;
    public uint MinConstantBufferOffsetAlignment;
    public uint MaxConstantBufferBindingSize;

    public uint MinStorageBufferOffsetAlignment;
    public uint MaxStorageBufferBindingSize;

    public ulong MaxBufferSize;
    public uint MaxPushConstantsSize;
    public uint MaxColorAttachments;
    public uint MaxViewports;

    public uint MaxVertexBuffers;
    public uint MaxVertexAttributes;
    public uint MaxVertexBufferArrayStride;

    public uint MaxComputeWorkgroupStorageSize;
    public uint MaxComputeInvocationsPerWorkGroup;
    public uint MaxComputeWorkGroupSizeX;
    public uint MaxComputeWorkGroupSizeY;
    public uint MaxComputeWorkGroupSizeZ;
    public uint MaxComputeWorkGroupsPerDimension;

    // VariableRateShading
    public uint VariableRateShadingTileSize;

    // Ray tracing
    public ulong RayTracingShaderGroupIdentifierSize;
    public ulong RayTracingShaderTableAligment;
    public ulong RayTracingShaderTableMaxStride;
    public uint RayTracingShaderRecursionMaxDepth;
    public uint RayTracingMaxGeometryCount;
}
