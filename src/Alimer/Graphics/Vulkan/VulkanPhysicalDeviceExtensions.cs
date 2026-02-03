// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

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
    // Core in 1.4
    public bool Maintenance5;
    public bool Maintenance6;
    public bool PushDescriptor;

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
    public bool ConservativeRasterization;
    public bool MemoryBudget;
    public bool AMD_DeviceCoherentMemory;
    public bool MemoryPriority;

    public bool ExternalMemory;
    public bool ExternalSemaphore;
    public bool ExternalFence;

    public bool DeferredHostOperations;
    public bool PortabilitySubset;
    public bool TextureCompressionASTC_HDR;
    public bool TextureCompressionASTC_3D;

    public bool ShaderViewportIndexLayer;
    public bool AccelerationStructure;
    public bool RaytracingPipeline;
    public bool RayQuery;
    public bool FragmentShadingRate;
    public bool MeshShader;
    public bool ConditionalRendering;
    public bool UnifiedImageLayouts;
    public bool Win32FullScreenExclusive;

    public VulkanPhysicalDeviceVideoExtensions Video;

    public static unsafe VulkanPhysicalDeviceExtensions Query(VkInstanceApi api, VkPhysicalDevice physicalDevice)
    {
        VkResult result = api.vkEnumerateDeviceExtensionProperties(physicalDevice, out uint count);
        if (result != VkResult.Success)
            return default;

        Span<VkExtensionProperties> vk_extensions = stackalloc VkExtensionProperties[(int)count];
        api.vkEnumerateDeviceExtensionProperties(physicalDevice, vk_extensions);

        VulkanPhysicalDeviceExtensions extensions = new();

        for (int i = 0; i < count; ++i)
        {
            fixed (byte* pExtensionName = vk_extensions[i].extensionName)
            {
                Utf8String extensionName = new(pExtensionName);

                if (extensionName == VK_KHR_MAINTENANCE_4_EXTENSION_NAME)
                {
                    extensions.Maintenance4 = true;
                }
                else if (extensionName == VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                {
                    extensions.DynamicRendering = true;
                }
                else if (extensionName == VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
                {
                    extensions.Synchronization2 = true;
                }
                else if (extensionName == VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME)
                {
                    extensions.ExtendedDynamicState = true;
                }
                else if (extensionName == VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME)
                {
                    extensions.ExtendedDynamicState2 = true;
                }
                else if (extensionName == VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME)
                {
                    extensions.PipelineCreationCacheControl = true;
                }
                else if (extensionName == VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME)
                {
                    extensions.FormatFeatureFlags2 = true;
                }
                else if (extensionName == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                {
                    extensions.Swapchain = true;
                }
                else if (extensionName == VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME)
                {
                    extensions.DepthClipEnable = true;
                }
                else if (extensionName == VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME)
                {
                    extensions.ConservativeRasterization = true;
                }
                else if (extensionName == VK_EXT_MEMORY_BUDGET_EXTENSION_NAME)
                {
                    extensions.MemoryBudget = true;
                }
                else if (extensionName == VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME)
                {
                    extensions.AMD_DeviceCoherentMemory = true;
                }
                else if (extensionName == VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME)
                {
                    extensions.MemoryPriority = true;
                }
                else if (extensionName == VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                {
                    extensions.DeferredHostOperations = true;
                }
                else if (extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
                {
                    extensions.PortabilitySubset = true;
                }
                else if (extensionName == VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME)
                {
                    extensions.TextureCompressionASTC_HDR = true;
                }
                else if (extensionName == VK_EXT_TEXTURE_COMPRESSION_ASTC_3D_EXTENSION_NAME)
                {
                    extensions.TextureCompressionASTC_3D = true;
                }
                else if (extensionName == VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME)
                {
                    extensions.ShaderViewportIndexLayer = true;
                }
                else if (extensionName == VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
                {
                    extensions.AccelerationStructure = true;
                }
                else if (extensionName == VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
                {
                    extensions.RaytracingPipeline = true;
                }
                else if (extensionName == VK_KHR_RAY_QUERY_EXTENSION_NAME)
                {
                    extensions.RayQuery = true;
                }
                else if (extensionName == VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)
                {
                    extensions.FragmentShadingRate = true;
                }
                else if (extensionName == VK_EXT_MESH_SHADER_EXTENSION_NAME)
                {
                    extensions.MeshShader = true;
                }
                else if (extensionName == VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME)
                {
                    extensions.ConditionalRendering = true;
                }
                else if (extensionName == VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME)
                {
                    extensions.UnifiedImageLayouts = true;
                }
                else if (extensionName == VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)
                {
                    extensions.Video.Queue = true;
                }
                else if (extensionName == VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)
                {
                    extensions.Video.DecodeQueue = true;
                }
                else if (extensionName == VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME)
                {
                    extensions.Video.DecodeH264 = true;
                }
                else if (extensionName == VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME)
                {
                    extensions.Video.DecodeH265 = true;
                }
                if (extensionName == VK_KHR_MAINTENANCE_5_EXTENSION_NAME)
                {
                    extensions.Maintenance5 = true;
                }
                else if (extensionName == VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME)
                {
                    extensions.Video.EncodeQueue = true;
                }
                else if (extensionName == VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME)
                {
                    extensions.Video.EncodeH264 = true;
                }
                else if (extensionName == VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME)
                {
                    extensions.Video.EncodeH265 = true;
                }

                if (OperatingSystem.IsWindows())
                {
                    if (extensionName == VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME)
                    {
                        extensions.Win32FullScreenExclusive = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)
                    {
                        extensions.ExternalMemory = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME)
                    {
                        extensions.ExternalSemaphore = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME)
                    {
                        extensions.ExternalFence = true;
                    }
                }
                else
                {
                    if (extensionName == VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME)
                    {
                        extensions.ExternalMemory = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)
                    {
                        extensions.ExternalSemaphore = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME)
                    {
                        extensions.ExternalFence = true;
                    }
                }
            }
        }

        VkPhysicalDeviceProperties2 properties2 = new();
        api.vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

        // Core 1.4
        if (properties2.properties.apiVersion >= VkVersion.Version_1_4)
        {
            extensions.Maintenance5 = true;
            extensions.Maintenance6 = true;
            extensions.PushDescriptor = true;
        }

        // Core 1.3
        if (properties2.properties.apiVersion >= VkVersion.Version_1_3)
        {
            extensions.Maintenance4 = true;
            extensions.DynamicRendering = true;
            extensions.Synchronization2 = true;
            extensions.ExtendedDynamicState = true;
            extensions.ExtendedDynamicState2 = true;
            extensions.PipelineCreationCacheControl = true;
            extensions.FormatFeatureFlags2 = true;
        }

        return extensions;
    }

}
