// Copyright (c) Amer Koleci and Contributors.
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
    // Core in 1.3
    public bool Maintenance4;
    public bool DynamicRendering;
    public bool Synchronization2;
    public bool ExtendedDynamicState;
    public bool ExtendedDynamicState2;
    public bool PipelineCreationCacheControl;
    public bool FormatFeatureFlags2;

    // Extensions
    public bool Swapchain;
    public bool DepthClipEnable;
    public bool MemoryBudget;
    public bool AMD_DeviceCoherentMemory;
    public bool MemoryPriority;

    public bool Maintenance5;
    public bool SupportsExternalSemaphore;
    public bool SupportsExternalMemory;

    public bool performance_query;
    public bool host_query_reset;
    public bool deferred_host_operations;
    public bool PortabilitySubset;
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
