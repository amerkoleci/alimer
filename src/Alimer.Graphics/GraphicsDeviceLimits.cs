// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    public struct GraphicsDeviceLimits
    {
        public uint MaxVertexAttributes;
        public uint MaxVertexBindings;
        public uint MaxVertexAttributeOffset;
        public uint MaxVertexBindingStride;
        public uint MaxTextureDimension1D;
        public uint MaxTextureDimension2D;
        public uint MaxTextureDimension3D;
        public uint MaxTextureDimensionCube;
        public uint MaxTextureArrayLayers;
        public uint MaxColorAttachments;
        public uint MaxUniformBufferRange;
        public uint MaxStorageBufferRange;
        public ulong MinUniformBufferOffsetAlignment;
        public ulong MinStorageBufferOffsetAlignment;
        public uint MaxSamplerAnisotropy;
        public uint MaxViewports;
        public uint MaxViewportWidth;
        public uint MaxViewportHeight;
        public uint MaxTessellationPatchSize;
        public uint MaxComputeSharedMemorySize;
        public uint MaxComputeWorkGroupCountX;
        public uint MaxComputeWorkGroupCountY;
        public uint MaxComputeWorkGroupCountZ;
        public uint MaxComputeWorkGroupInvocations;
        public uint MaxComputeWorkGroupSizeX;
        public uint MaxComputeWorkGroupSizeY;
        public uint MaxComputeWorkGroupSizeZ;
    }
}
