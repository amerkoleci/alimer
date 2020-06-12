// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceFeatures
    {
        public readonly Bool32 IndependentBlend;
        public readonly Bool32 ComputeShader;
        public readonly Bool32 TessellationShader;
        public readonly Bool32 MultiViewport;
        public readonly Bool32 IndexUInt32;
        public readonly Bool32 multiDrawIndirect;
        public readonly Bool32 FillModeNonSolid;
        public readonly Bool32 SamplerAnisotropy;
        public readonly Bool32 TextureCompressionETC2;
        public readonly Bool32 TextureCompressionASTC_LDR;
        public readonly Bool32 TextureCompressionBC;
        public readonly Bool32 TextureCubeArray;
        public readonly Bool32 Raytracing;
    }


    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceLimits
    {
        public readonly uint max_vertex_attributes;
        public readonly uint max_vertex_bindings;
        public readonly uint max_vertex_attribute_offset;
        public readonly uint max_vertex_binding_stride;
        public readonly uint max_texture_size_1d;
        public readonly uint max_texture_size_2d;
        public readonly uint max_texture_size_3d;
        public readonly uint max_texture_size_cube;
        public readonly uint max_texture_array_layers;
        public readonly uint max_color_attachments;
        public readonly uint max_uniform_buffer_size;
        public readonly ulong min_uniform_buffer_offset_alignment;
        public readonly uint max_storage_buffer_size;
        public readonly ulong min_storage_buffer_offset_alignment;
        public readonly uint max_sampler_anisotropy;
        public readonly uint max_viewports;
        public readonly uint max_viewport_width;
        public readonly uint max_viewport_height;
        public readonly uint max_tessellation_patch_size;
        public readonly float point_size_range_min;
        public readonly float point_size_range_max;
        public readonly float line_width_range_min;
        public readonly float line_width_range_max;
        public readonly uint max_compute_shared_memory_size;
        public readonly uint max_compute_work_group_count_x;
        public readonly uint max_compute_work_group_count_y;
        public readonly uint max_compute_work_group_count_z;
        public readonly uint max_compute_work_group_invocations;
        public readonly uint max_compute_work_group_size_x;
        public readonly uint max_compute_work_group_size_y;
        public readonly uint max_compute_work_group_size_z;
    }

    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceCaps
    {
        public readonly BackendType BackendType;
        public readonly uint VendorId;
        public readonly uint DeviceId;
        public readonly GPUDeviceFeatures Features;
        public readonly GPUDeviceLimits Limits;
    }
}
