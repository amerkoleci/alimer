// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines the <see cref="GPUDevice"/> ray tracing limits and capabilities.
/// </summary>
public struct RayTracingLimits
{
    public ulong ShaderGroupIdentifierSize;
    public ulong ShaderTableAlignment;
    public ulong ShaderTableMaxStride;
    public uint ShaderRecursionMaxDepth;
    public uint MaxGeometryCount;
    public uint MinAccelerationStructureScratchOffsetAlignment;
}

/// <summary>
/// Defines <see cref="GPUDevice"/> limits and capabilities.
/// These limits are determined by the underlying graphics API and the GPU hardware.
/// </summary>
public struct GPUDeviceLimits
{
    public uint MaxTextureDimension1D;
    public uint MaxTextureDimension2D;
    public uint MaxTextureDimension3D;
    public uint MaxTextureDimensionCube;
    public uint MaxTextureArrayLayers;
    //public uint MaxTexelBufferDimension2D;

    public ulong MinConstantBufferOffsetAlignment;
    public uint MaxConstantBufferBindingSize;

    public ulong MinStorageBufferOffsetAlignment;
    public uint MaxStorageBufferBindingSize;

    public ulong TextureRowPitchAlignment;
    public ulong TextureDepthPitchAlignment;
    public ulong MinLinearAllocatorOffsetAlignment;

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

    /// <summary>
    /// Highest supported shader model
    /// </summary>
    public ShaderModel HighestShaderModel;

    // ConservativeRasterization
    public ConservativeRasterizationTier ConservativeRasterizationTier;

    // ProgrammableSamplePositions
    public ProgrammableSamplePositionsTier ProgrammableSamplePositionsTier;

    // VariableRateShading
    public VariableShadingRateTier VariableShadingRateTier;
    public uint VariableShadingRateImageTileSize;
    public bool IsAdditionalVariableShadingRatesSupported;

    // Ray tracing
    public RayTracingTier RayTracingTier;
    public RayTracingLimits RayTracing;

    // Mesh shader
    public MeshShaderTier MeshShaderTier;
}
