// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics;

public readonly struct GraphicsDeviceLimits
{
    public uint MaxVertexAttributes { get; init; }
    public uint MaxVertexBindings { get; init; }
    public uint MaxVertexAttributeOffset { get; init; }
    public uint MaxVertexBindingStride { get; init; }
    public uint MaxTextureDimension1D { get; init; }
    public uint MaxTextureDimension2D { get; init; }
    public uint MaxTextureDimension3D { get; init; }
    public uint MaxTextureDimensionCube { get; init; }
    public uint MaxTextureArrayLayers { get; init; }
    public uint MaxColorAttachments { get; init; }
    public uint MaxUniformBufferRange { get; init; }
    public uint MaxStorageBufferRange { get; init; }
    public ulong MinUniformBufferOffsetAlignment { get; init; }
    public ulong MinStorageBufferOffsetAlignment { get; init; }
    public uint MaxSamplerAnisotropy { get; init; }
    public uint MaxViewports { get; init; }
    public uint MaxViewportWidth { get; init; }
    public uint MaxViewportHeight { get; init; }
    public uint MaxTessellationPatchSize { get; init; }
    public uint MaxComputeSharedMemorySize { get; init; }
    public uint MaxComputeWorkGroupCountX { get; init; }
    public uint MaxComputeWorkGroupCountY { get; init; }
    public uint MaxComputeWorkGroupCountZ { get; init; }
    public uint MaxComputeWorkGroupInvocations { get; init; }
    public uint MaxComputeWorkGroupSizeX { get; init; }
    public uint MaxComputeWorkGroupSizeY { get; init; }
    public uint MaxComputeWorkGroupSizeZ { get; init; }
}
