// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public struct GraphicsDeviceRayTracingLimits
{
    public ulong ShaderGroupIdentifierSize;
    public ulong ShaderTableAlignment;
    public ulong ShaderTableMaxStride;
    public uint ShaderRecursionMaxDepth;
    public uint MaxGeometryCount;
    public uint MinAccelerationStructureScratchOffsetAlignment;
}

public struct GraphicsDeviceLimits
{
    public uint MaxTextureDimension1D;
    public uint MaxTextureDimension2D;
    public uint MaxTextureDimension3D;
    public uint MaxTextureDimensionCube;
    public uint MaxTextureArrayLayers;
    //public uint MaxTexelBufferDimension2D;

    public uint MinConstantBufferOffsetAlignment;
    public uint MaxConstantBufferBindingSize;

    public uint MinStorageBufferOffsetAlignment;
    public uint MaxStorageBufferBindingSize;

    public uint TextureRowPitchAlignment;
    public uint TextureDepthPitchAlignment;

    public ulong MaxBufferSize;
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
    public VariableShadingRateTier VariableShadingRateTier;
    public uint VariableShadingRateImageTileSize;
    public bool IsAdditionalVariableShadingRatesSupported;

    // Ray tracing
    public RayTracingTier RayTracingTier;
    public GraphicsDeviceRayTracingLimits RayTracing;

    // Mesh shader
    public MeshShaderTier MeshShaderTier;
}
