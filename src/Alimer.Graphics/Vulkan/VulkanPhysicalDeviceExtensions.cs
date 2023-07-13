// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Vulkan;

internal struct VulkanPhysicalDeviceVideoExtensions
{
    public bool Queue;
    public bool DecodeQueue;
    public bool DecodeH264;
    public bool DecodeH265;
    public bool EncodeQueue;
    public bool EncodeH264;
    public bool EncodeH265;
}

internal struct VulkanPhysicalDeviceExtensions
{
    public bool Swapchain;
    // Core in 1.2
    public bool driverProperties;
    public bool renderPass2;
    public bool samplerFilterMinMax;
    public bool depthStencilResolve;
    // Core in 1.3
    public bool maintenance4;
    public bool dynamicRendering;
    public bool synchronization2;
    public bool extended_dynamic_state;
    public bool extended_dynamic_state2;
    public bool pipeline_creation_cache_control;
    public bool format_feature_flags2;

    // Extensions
    public bool DepthClipEnable;
    public bool MemoryBudget;
    public bool AMD_DeviceCoherentMemory;
    public bool MemoryPriority;

    public bool SupportsExternalSemaphore;
    public bool SupportsExternalMemory;

    public bool performance_query;
    public bool host_query_reset;
    public bool deferred_host_operations;
    public bool portability_subset;
    public bool accelerationStructure;
    public bool raytracingPipeline;
    public bool rayQuery;
    public bool FragmentShadingRate;
    public bool MeshShader;
    public bool ConditionalRendering;
    public bool win32_full_screen_exclusive;

    public readonly bool SupportsExternal => SupportsExternalSemaphore && SupportsExternalMemory;

    public VulkanPhysicalDeviceVideoExtensions Video;
}
