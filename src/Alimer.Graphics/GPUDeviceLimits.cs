// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceLimits
    {
        public readonly uint MaxVertexAttributes;
        public readonly uint MaxVertexBindings;
        public readonly uint MaxVertexAttributeOffset;
        public readonly uint MaxVertexBindingStride;
        public readonly uint MaxTextureDimension2D;
        public readonly uint MaxTextureDimension3D;
        public readonly uint MaxTextureDimensionCube;
        public readonly uint MaxTextureArrayLayers;
        public readonly uint MaxColorAttachments;
        public readonly uint MaxUniformBufferRange;
        public readonly uint MaxStorageBufferRange;
        public readonly ulong MinUniformBufferOffsetAlignment;
        public readonly ulong MinStorageBufferOffsetAlignment;
        public readonly uint MaxSamplerAnisotropy;
        public readonly uint MaxViewports;
        public readonly uint MaxViewportWidth;
        public readonly uint MaxViewportHeight;
        public readonly uint MaxTessellationPatchSize;
        public readonly uint MaxComputeSharedMemorySize;
        public readonly uint MaxComputeWorkGroupCountX;
        public readonly uint MaxComputeWorkGroupCountY;
        public readonly uint MaxComputeWorkGroupCountZ;
        public readonly uint MaxComputeWorkGroupInvocations;
        public readonly uint MaxComputeWorkGroupSizeX;
        public readonly uint MaxComputeWorkGroupSizeY;
        public readonly uint MaxComputeWorkGroupSizeZ;
    }
}
