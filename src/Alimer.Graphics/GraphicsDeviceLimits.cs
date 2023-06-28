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

    public ulong MaxConstantBufferBindingSize;
    public ulong MaxStorageBufferBindingSize;
    public uint MinUniformBufferOffsetAlignment;
    public uint MinStorageBufferOffsetAlignment;
    public uint MaxVertexBuffers;
    public uint MaxVertexAttributes;
    public uint MaxVertexBufferArrayStride;
    public uint MaxComputeWorkgroupStorageSize;
    public uint MaxComputeInvocationsPerWorkGroup;
    public uint MaxComputeWorkGroupSizeX;
    public uint MaxComputeWorkGroupSizeY;
    public uint MaxComputeWorkGroupSizeZ;
    public uint MaxComputeWorkGroupsPerDimension;
    public uint MaxViewports;
    public uint MaxViewportDimensions1;
    public uint MaxViewportDimensions2;
    public uint MaxColorAttachments;
}
