// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Utilities;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using XenoAtom.Collections;
using static Alimer.Graphics.Vulkan.Vma;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanGraphicsDevice : GraphicsDevice
{
    private const ulong TimeoutValue = 2000000000ul; // 2 seconds

    private readonly VulkanGraphicsAdapter _adapter;
    private readonly uint[] _queueFamilyIndices;
    private readonly uint[] _queueIndices;
    private readonly uint[] _queueCounts;

    private readonly VkPhysicalDevice _physicalDevice = VkPhysicalDevice.Null;
    private readonly VkDevice _handle = VkDevice.Null;
    private readonly VkDeviceApi _deviceApi;
    private readonly VulkanCopyAllocator _copyAllocator;
    private readonly VulkanCommandQueue[] _queues = new VulkanCommandQueue[(int)QueueType.Count];
    private readonly VmaAllocator _allocator;
    private readonly VmaAllocation _nullBufferAllocation = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation1D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation2D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation3D = VmaAllocation.Null;

    private readonly Dictionary<SamplerDescription, VkSampler> _samplerCache = [];

    private readonly VkBuffer _nullBuffer = default;
    private readonly VkBufferView _nullBufferView = default;
    private readonly VkImage _nullImage1D = default;
    private readonly VkImage _nullImage2D = default;
    private readonly VkImage _nullImage3D = default;
    private readonly VkImageView _nullImageView1D = default;
    private readonly VkImageView _nullImageView1DArray = default;
    private readonly VkImageView _nullImageView2D = default;
    private readonly VkImageView _nullImageView2DArray = default;
    private readonly VkImageView _nullImageViewCube = default;
    private readonly VkImageView _nullImageViewCubeArray = default;
    private readonly VkImageView _nullImageView3D = default;
    private readonly VkSampler _nullSampler = default;

    public VulkanGraphicsDevice(VulkanGraphicsAdapter adapter, in GraphicsDeviceDescription description)
        : base(description)
    {
        _adapter = adapter;
        _physicalDevice = adapter.Handle;

        _queueFamilyIndices = new uint[(int)QueueType.Count];
        _queueIndices = new uint[(int)QueueType.Count];
        _queueCounts = new uint[(int)QueueType.Count];
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            _queueFamilyIndices[i] = VK_QUEUE_FAMILY_IGNORED;
        }

        VkInstanceApi instanceApi = _adapter.VkGraphicsManager.InstanceApi;

        instanceApi.vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, out uint count);

        VkQueueFamilyProperties2* queueProps = stackalloc VkQueueFamilyProperties2[(int)count];
        VkQueueFamilyVideoPropertiesKHR* queueFamiliesVideo = stackalloc VkQueueFamilyVideoPropertiesKHR[(int)count];
        for (int i = 0; i < count; ++i)
        {
            queueProps[i] = new();

            if (_adapter.Extensions.Video.Queue)
            {
                queueProps[i].pNext = &queueFamiliesVideo[i];
                queueFamiliesVideo[i] = new();
            }
        }

        instanceApi.vkGetPhysicalDeviceQueueFamilyProperties2(
            _physicalDevice,
            &count,
            queueProps
            );
        int queueFamilyCount = (int)count;

        VkSurfaceKHR surface = VkSurfaceKHR.Null;

        uint* offsets = stackalloc uint[queueFamilyCount];
        float* priorities = stackalloc float[queueFamilyCount * (int)QueueType.Count];
        bool FindVacantQueue(VkQueueFlags required, VkQueueFlags ignored, float priority, ref uint family, ref uint index)
        {
            for (uint i = 0; i < queueFamilyCount; i++)
            {
                // Skip queues with undesired flags
                if ((queueProps[i].queueFamilyProperties.queueFlags & ignored) != 0)
                    continue;

                // Check for present on graphics queues
                if ((required & VkQueueFlags.Graphics) != 0 && surface != VkSurfaceKHR.Null)
                {
                    VkBool32 presentSupport = false;
                    if (surface != VkSurfaceKHR.Null)
                    {
                        if (instanceApi.vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &presentSupport) != VkResult.Success)
                            continue;
                    }
                    else
                    {
                        if (OperatingSystem.IsWindows())
                        {
                            presentSupport = instanceApi.vkGetPhysicalDeviceWin32PresentationSupportKHR(_physicalDevice, i);
                        }
                        else if (OperatingSystem.IsAndroid())
                        {
                            // All Android queues surfaces support present.
                            presentSupport = true;
                        }
                    }

                    if (!presentSupport)
                        continue;
                }

                if ((required & VkQueueFlags.VideoDecodeKHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[i].videoCodecOperations;

                    if ((videoCodecOperations & VkVideoCodecOperationFlagsKHR.DecodeH264) == 0 &&
                        (videoCodecOperations & VkVideoCodecOperationFlagsKHR.DecodeH265) == 0)
                    {
                        continue;
                    }
                }

                if ((required & VkQueueFlags.VideoEncodeKHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[i].videoCodecOperations;

                    if ((videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH264) == 0 &&
                        (videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH265) == 0)
                    {
                        continue;
                    }
                }


                if (queueProps[i].queueFamilyProperties.queueCount > 0 &&
                    (queueProps[i].queueFamilyProperties.queueFlags & required) == required)
                {
                    family = i;
                    queueProps[i].queueFamilyProperties.queueCount--;
                    index = offsets[i]++;
                    priorities[i * (int)QueueType.Count + index] = priority;
                    return true;
                }
            }
            return false;
        }

        // Find graphics queue
        if (!FindVacantQueue(VkQueueFlags.Graphics | VkQueueFlags.Compute,
            VkQueueFlags.None, 0.5f,
            ref _queueFamilyIndices[(int)QueueType.Graphics],
            ref _queueIndices[(int)QueueType.Graphics]))
        {
            throw new GraphicsException("Vulkan: Could not find graphics queue with compute and present");
        }

        // Prefer another graphics queue since we can do async graphics that way.
        // The compute queue is to be treated as high priority since we also do async graphics on it.
        if (!FindVacantQueue(VkQueueFlags.Graphics | VkQueueFlags.Compute, VkQueueFlags.None, 1.0f, ref _queueFamilyIndices[(int)QueueType.Compute], ref _queueIndices[(int)QueueType.Compute]) &&
            !FindVacantQueue(VkQueueFlags.Compute, VkQueueFlags.None, 1.0f, ref _queueFamilyIndices[(int)QueueType.Compute], ref _queueIndices[(int)QueueType.Compute]))
        {
            _queueFamilyIndices[(int)QueueType.Compute] = _queueFamilyIndices[(int)QueueType.Graphics];
            _queueIndices[(int)QueueType.Compute] = _queueIndices[(int)QueueType.Graphics];
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(VkQueueFlags.Transfer, VkQueueFlags.Graphics | VkQueueFlags.Compute, 0.5f, ref _queueFamilyIndices[(int)QueueType.Copy], ref _queueIndices[(int)QueueType.Copy]) &&
            !FindVacantQueue(VkQueueFlags.Compute, VkQueueFlags.Graphics, 0.5f, ref _queueFamilyIndices[(int)QueueType.Copy], ref _queueIndices[(int)QueueType.Copy]))
        {
            _queueFamilyIndices[(int)QueueType.Copy] = _queueFamilyIndices[(int)QueueType.Compute];
            _queueIndices[(int)QueueType.Copy] = _queueIndices[(int)QueueType.Compute];
        }

        if (_adapter.Extensions.Video.Queue)
        {
            if (!FindVacantQueue(VkQueueFlags.VideoDecodeKHR, 0, 0.5f, ref _queueFamilyIndices[(int)QueueType.VideoDecode], ref _queueIndices[(int)QueueType.VideoDecode]))
            {
                _queueFamilyIndices[(int)QueueType.VideoDecode] = VK_QUEUE_FAMILY_IGNORED;
                _queueIndices[(int)QueueType.VideoDecode] = uint.MaxValue;
            }

            if (!FindVacantQueue(VkQueueFlags.VideoEncodeKHR, 0, 0.5f, ref _queueFamilyIndices[(int)QueueType.VideoEncode], ref _queueIndices[(int)QueueType.VideoEncode]))
            {
                _queueFamilyIndices[(int)QueueType.VideoEncode] = VK_QUEUE_FAMILY_IGNORED;
                _queueIndices[(int)QueueType.VideoEncode] = uint.MaxValue;
            }
        }

        uint queueCreateInfosCount = 0u;
        VkDeviceQueueCreateInfo* queueCreateInfos = stackalloc VkDeviceQueueCreateInfo[queueFamilyCount];
        for (uint i = 0; i < queueFamilyCount; i++)
        {
            if (offsets[i] == 0)
                continue;

            VkDeviceQueueCreateInfo queueCreateInfo = new()
            {
                queueFamilyIndex = i,
                queueCount = offsets[i],
                pQueuePriorities = &priorities[i * (int)QueueType.Count]
            };
            queueCreateInfos[queueCreateInfosCount] = queueCreateInfo;
            queueCreateInfosCount++;

            _queueCounts[i] = offsets[_queueFamilyIndices[i]];
        }

        // Setup extensions and features
        UnsafeList<Utf8String> enabledDeviceExtensions = [];
        enabledDeviceExtensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDeviceFeatures2 features2 = _adapter.Features2;
        VkPhysicalDeviceVulkan11Features features11 = _adapter.Features11;
        VkPhysicalDeviceVulkan12Features features12 = _adapter.Features12;
        VkPhysicalDeviceVulkan13Features features13 = _adapter.Features13;
        VkPhysicalDeviceVulkan14Features features14 = _adapter.Features14;

        VkBaseOutStructure* featureChainCurrent = (VkBaseOutStructure*)&features2;
        AddToFeatureChain(&features11);
        AddToFeatureChain(&features12);
        if (_adapter.ApiVersion >= VK_API_VERSION_1_3)
        {
            AddToFeatureChain(&features13);
        }

        if (_adapter.ApiVersion >= VK_API_VERSION_1_4)
        {
            AddToFeatureChain(&features14);
        }

        // Core in 1.3
        if (_adapter.ApiVersion < VK_API_VERSION_1_3)
        {
            if (_adapter.Extensions.Maintenance4)
            {
                enabledDeviceExtensions.Add(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

                VkPhysicalDeviceMaintenance4Features maintenance4Features = _adapter.Maintenance4Features;
                AddToFeatureChain(&maintenance4Features);
            }

            if (_adapter.Extensions.DynamicRendering)
            {
                enabledDeviceExtensions.Add(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

                var dynamicRenderingFeatures = _adapter.DynamicRenderingFeatures;
                AddToFeatureChain(&dynamicRenderingFeatures);
            }

            if (_adapter.Extensions.Synchronization2)
            {
                enabledDeviceExtensions.Add(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

                var synchronization2Features = _adapter.Synchronization2Features;
                AddToFeatureChain(&synchronization2Features);
            }

            if (_adapter.Extensions.ExtendedDynamicState)
            {
                enabledDeviceExtensions.Add(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

                var extendedDynamicStateFeatures = _adapter.ExtendedDynamicStateFeatures;
                AddToFeatureChain(&extendedDynamicStateFeatures);
            }

            if (_adapter.Extensions.ExtendedDynamicState2)
            {
                enabledDeviceExtensions.Add(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

                var extendedDynamicState2Features = _adapter.ExtendedDynamicState2Features;
                AddToFeatureChain(&extendedDynamicState2Features);
            }

            if (_adapter.Extensions.PipelineCreationCacheControl)
            {
                enabledDeviceExtensions.Add(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);

                var pipelineCreationCacheControlFeatures = _adapter.PipelineCreationCacheControlFeatures;
                AddToFeatureChain(&pipelineCreationCacheControlFeatures);
            }

            if (_adapter.Extensions.TextureCompressionAstcHdr)
            {
                enabledDeviceExtensions.Add(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

                var astcHdrFeatures = _adapter.AstcHdrFeatures;
                AddToFeatureChain(&astcHdrFeatures);
            }
        }
        else
        {
            // Core in 1.4
            if (_adapter.ApiVersion < VK_API_VERSION_1_4)
            {
                if (_adapter.Extensions.Maintenance5)
                {
                    enabledDeviceExtensions.Add(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);

                    var maintenance5Features = _adapter.Maintenance5Features;
                    AddToFeatureChain(&maintenance5Features);
                }

                if (_adapter.Extensions.Maintenance6)
                {
                    enabledDeviceExtensions.Add(VK_KHR_MAINTENANCE_6_EXTENSION_NAME);

                    var maintenance6Features = _adapter.Maintenance6Features;
                    AddToFeatureChain(&maintenance6Features);
                }

                if (_adapter.Extensions.PushDescriptor)
                {
                    enabledDeviceExtensions.Add(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
                }
            }
        }


        if (_adapter.Extensions.MemoryBudget)
        {
            enabledDeviceExtensions.Add(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        }

        if (_adapter.Extensions.AMD_DeviceCoherentMemory)
        {
            enabledDeviceExtensions.Add(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
        }

        if (_adapter.Extensions.MemoryPriority)
        {
            enabledDeviceExtensions.Add(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
        }

        if (_adapter.Extensions.DeferredHostOperations)
        {
            enabledDeviceExtensions.Add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        }

        if (_adapter.Extensions.PortabilitySubset)
        {
            enabledDeviceExtensions.Add(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
        }

        if (_adapter.Extensions.DepthClipEnable)
        {
            enabledDeviceExtensions.Add(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

            VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = _adapter.DepthClipEnableFeatures;
            AddToFeatureChain(&depthClipEnableFeatures);
        }

        if (_adapter.Extensions.ShaderViewportIndexLayer)
        {
            enabledDeviceExtensions.Add(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        }

        if (_adapter.Extensions.ExternalMemory)
        {
            if (OperatingSystem.IsWindows())
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
            }
            else
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
            }
        }

        if (_adapter.Extensions.ExternalSemaphore)
        {
            if (OperatingSystem.IsWindows())
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
            }
            else
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
            }
        }

        if (_adapter.Extensions.ExternalFence)
        {
            if (OperatingSystem.IsWindows())
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
            }
            else
            {
                enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
            }
        }

        if (_adapter.Extensions.AccelerationStructure)
        {
            Guard.IsTrue(_adapter.Extensions.DeferredHostOperations);

            // Required by VK_KHR_acceleration_structure
            enabledDeviceExtensions.Add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            enabledDeviceExtensions.Add(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = _adapter.AccelerationStructureFeatures;
            AddToFeatureChain(&accelerationStructureFeatures);

            if (_adapter.Extensions.RaytracingPipeline)
            {
                // Required by VK_KHR_pipeline_library
                enabledDeviceExtensions.Add(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                enabledDeviceExtensions.Add(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

                VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = _adapter.RayTracingPipelineFeatures;
                AddToFeatureChain(&rayTracingPipelineFeatures);
            }

            if (_adapter.Extensions.RayQuery)
            {
                enabledDeviceExtensions.Add(VK_KHR_RAY_QUERY_EXTENSION_NAME);

                VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = _adapter.RayQueryFeatures;
                AddToFeatureChain(&rayQueryFeatures);
            }
        }

        if (_adapter.Extensions.FragmentShadingRate)
        {
            enabledDeviceExtensions.Add(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

            VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = _adapter.FragmentShadingRateFeatures;
            AddToFeatureChain(&fragmentShadingRateFeatures);
        }

        if (_adapter.Extensions.MeshShader)
        {
            enabledDeviceExtensions.Add(VK_EXT_MESH_SHADER_EXTENSION_NAME);

            VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = _adapter.MeshShaderFeatures;
            AddToFeatureChain(&meshShaderFeatures);
        }

        if (_adapter.Extensions.ConditionalRendering)
        {
            enabledDeviceExtensions.Add(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

            VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = _adapter.ConditionalRenderingFeatures;
            AddToFeatureChain(&conditionalRenderingFeatures);
        }

        if (_adapter.Extensions.Video.Queue)
        {
            enabledDeviceExtensions.Add(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

            if (_adapter.Extensions.Video.DecodeQueue)
            {
                enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

                if (_adapter.Extensions.Video.DecodeH264)
                {
                    enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
                }

                if (_adapter.Extensions.Video.DecodeH265)
                {
                    enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
                }
            }

#if VULKAN_VIDEO_ENCODE
                if (physicalDeviceExtensions.video.encode_queue)
                {
                    enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

                    if (physicalDeviceExtensions.video.encode_h264)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.video.encode_h265)
                    {
                        enabledDeviceExtensions.push_back(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME);
                    }
                }
#endif // VULKAN_VIDEO_ENCODE

        }

        using Utf8StringArray deviceExtensionNames = new(enabledDeviceExtensions);
        VkDeviceCreateInfo createInfo = new()
        {
            pNext = &features2,
            queueCreateInfoCount = queueCreateInfosCount,
            pQueueCreateInfos = queueCreateInfos,
            enabledExtensionCount = deviceExtensionNames.Length,
            ppEnabledExtensionNames = deviceExtensionNames,
            pEnabledFeatures = null,
        };

        VkResult result = _adapter.VkGraphicsManager.InstanceApi.vkCreateDevice(
            PhysicalDevice,
            &createInfo,
            null,
            out _handle
            );
        if (result != VkResult.Success)
        {
            throw new GraphicsException($"Failed to create Vulkan Logical Device, {result}");
        }

        _deviceApi = GetApi(_adapter.VkGraphicsManager.Instance, _handle);

#if DEBUG
        Log.Info($"Enabled {createInfo.enabledExtensionCount} Device Extensions:");
        for (uint i = 0; i < createInfo.enabledExtensionCount; ++i)
        {
            string extensionName = VkStringInterop.ConvertToManaged(createInfo.ppEnabledExtensionNames[i])!;
            Log.Info($"\t{extensionName}");
        }
#endif

        // Core in 1.1
        VmaAllocatorCreateFlags allocatorFlags =
            VmaAllocatorCreateFlags.KHRDedicatedAllocation | VmaAllocatorCreateFlags.KHRBindMemory2;

        if (_adapter.Extensions.MemoryBudget)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.EXTMemoryBudget;
        }

        if (_adapter.Extensions.AMD_DeviceCoherentMemory)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.AMDDeviceCoherentMemory;
        }

        if (_adapter.Features12.bufferDeviceAddress)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.BufferDeviceAddress;
        }

        if (_adapter.Extensions.MemoryPriority)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.EXTMemoryPriority;
        }

        if (_adapter.ApiVersion < VkVersion.Version_1_3)
        {
            //if (_adapter.Maintenance4Features.maintenance4)
            //{
            //    allocatorFlags |= VmaAllocatorCreateFlags.KHRMaintenance4;
            //}
        }

        if (_adapter.Extensions.Maintenance5)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.KHRMaintenance5;
        }

        VmaAllocatorCreateInfo allocatorCreateInfo = new()
        {
            physicalDevice = PhysicalDevice,
            device = _handle,
            instance = _adapter.VkGraphicsManager.Instance,
            vulkanApiVersion = VkVersion.Version_1_3,
            flags = allocatorFlags,
        };
        vmaCreateAllocator(in allocatorCreateInfo, out _allocator).CheckResult();

        // Queues
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queueFamilyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
            {
                _queues[i] = new VulkanCommandQueue(this, (QueueType)i);
            }
        }

        _copyAllocator = new(this);

        // Pipeline Cache
        {
            // TODO: Add cache from disk
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo = new();

            _deviceApi.vkCreatePipelineCache(_handle, in pipelineCacheCreateInfo, out VkPipelineCache pipelineCache).CheckResult();
            PipelineCache = pipelineCache;
        }

        // Create default null descriptors
        {
            VkBufferCreateInfo bufferInfo = new()
            {
                flags = 0,
                size = 4,
                usage = VkBufferUsageFlags.UniformBuffer | VkBufferUsageFlags.UniformTexelBuffer | VkBufferUsageFlags.StorageTexelBuffer | VkBufferUsageFlags.StorageBuffer | VkBufferUsageFlags.VertexBuffer,
            };

            VmaAllocationCreateInfo allocInfo = new()
            {
                preferredFlags = VkMemoryPropertyFlags.DeviceLocal
            };
            vmaCreateBuffer(_allocator, &bufferInfo, &allocInfo, out _nullBuffer, out _nullBufferAllocation).CheckResult();

            VkBufferViewCreateInfo viewInfo = new()
            {
                format = VkFormat.R32G32B32A32Sfloat,
                range = VK_WHOLE_SIZE,
                buffer = _nullBuffer
            };
            _deviceApi.vkCreateBufferView(_handle, &viewInfo, null, out _nullBufferView).CheckResult();

            VkImageCreateInfo imageInfo = new();
            imageInfo.extent.width = 1;
            imageInfo.extent.height = 1;
            imageInfo.extent.depth = 1;
            imageInfo.format = VkFormat.R8G8B8A8Unorm;
            imageInfo.arrayLayers = 1;
            imageInfo.mipLevels = 1;
            imageInfo.samples = VkSampleCountFlags.Count1;
            imageInfo.initialLayout = VkImageLayout.Undefined;
            imageInfo.tiling = VkImageTiling.Optimal;
            imageInfo.usage = VkImageUsageFlags.Sampled | VkImageUsageFlags.Storage;
            imageInfo.flags = 0;

            allocInfo.usage = VmaMemoryUsage.GpuOnly;

            imageInfo.imageType = VkImageType.Image1D;
            vmaCreateImage(_allocator, &imageInfo, &allocInfo, out _nullImage1D, out _nullImageAllocation1D).CheckResult();

            imageInfo.imageType = VkImageType.Image2D;
            imageInfo.flags = VkImageCreateFlags.CubeCompatible;
            imageInfo.arrayLayers = 6;
            vmaCreateImage(_allocator, &imageInfo, &allocInfo, out _nullImage2D, out _nullImageAllocation2D).CheckResult();

            imageInfo.imageType = VkImageType.Image3D;
            imageInfo.flags = 0;
            imageInfo.arrayLayers = 1;
            vmaCreateImage(_allocator, &imageInfo, &allocInfo, out _nullImage3D, out _nullImageAllocation3D).CheckResult();

            // Transitions:
            {
                VulkanUploadContext uploadContext = Allocate(0);

                VkImageMemoryBarrier2 barrier = new()
                {
                    oldLayout = imageInfo.initialLayout,
                    newLayout = VkImageLayout.General,
                    srcStageMask = VkPipelineStageFlags2.Transfer,
                    srcAccessMask = 0,
                    dstStageMask = VkPipelineStageFlags2.AllCommands,
                    dstAccessMask = VkAccessFlags2.ShaderRead | VkAccessFlags2.ShaderWrite,
                    subresourceRange = new VkImageSubresourceRange(VkImageAspectFlags.Color, 0, 1, 0, 1),
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    image = _nullImage1D,
                };

                VkDependencyInfo dependencyInfo = new()
                {
                    imageMemoryBarrierCount = 1,
                    pImageMemoryBarriers = &barrier
                };
                _deviceApi.vkCmdPipelineBarrier2(uploadContext.TransitionCommandBuffer, &dependencyInfo);

                barrier.image = _nullImage2D;
                barrier.subresourceRange.layerCount = 6;
                _deviceApi.vkCmdPipelineBarrier2(uploadContext.TransitionCommandBuffer, &dependencyInfo);

                barrier.image = _nullImage3D;
                barrier.subresourceRange.layerCount = 1;
                _deviceApi.vkCmdPipelineBarrier2(uploadContext.TransitionCommandBuffer, &dependencyInfo);

                Submit(in uploadContext);
            }

            VkImageViewCreateInfo imageViewInfo = new();
            imageViewInfo.subresourceRange.aspectMask = VkImageAspectFlags.Color;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.format = VkFormat.R8G8B8A8Unorm;
            imageViewInfo.image = _nullImage1D;
            imageViewInfo.viewType = VkImageViewType.Image1D;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageView1D).CheckResult();

            imageViewInfo.image = _nullImage1D;
            imageViewInfo.viewType = VkImageViewType.Image1DArray;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageView1DArray).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.Image2D;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageView2D).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.Image2DArray;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageView2DArray).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.ImageCube;
            imageViewInfo.subresourceRange.layerCount = 6;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageViewCube).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.ImageCubeArray;
            imageViewInfo.subresourceRange.layerCount = 6;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageViewCubeArray).CheckResult();

            imageViewInfo.image = _nullImage3D;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.viewType = VkImageViewType.Image3D;
            _deviceApi.vkCreateImageView(_handle, &imageViewInfo, null, out _nullImageView3D).CheckResult();

            _nullSampler = GetOrCreateVulkanSampler(new SamplerDescription());
        }

        TimestampFrequency = (ulong)(1.0 / _adapter.Properties2.properties.limits.timestampPeriod * 1000 * 1000 * 1000);

        void AddToFeatureChain(void* next)
        {
            VkBaseOutStructure* n = (VkBaseOutStructure*)next;
            featureChainCurrent->pNext = n;
            featureChainCurrent = n;
        }
    }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public VkInstanceApi InstanceApi => _adapter.VkGraphicsManager.InstanceApi;
    public VulkanGraphicsAdapter VkAdapter => _adapter;
    public bool DebugUtils => _adapter.VkGraphicsManager.DebugUtils;

    public VkPhysicalDevice PhysicalDevice => _physicalDevice;
    public uint GraphicsFamily => _queueFamilyIndices[(int)QueueType.Graphics];
    public uint ComputeFamily => _queueFamilyIndices[(int)QueueType.Compute];
    public uint CopyQueueFamily => _queueFamilyIndices[(int)QueueType.Copy];
    public uint VideoDecodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoDecode];
    public uint VideoEncodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoEncode];

    public VkDevice Handle => _handle;
    public VkDeviceApi DeviceApi => _deviceApi;
    public VulkanCommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public VulkanCommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public VulkanCommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public VulkanCommandQueue? VideoDecodeQueue => _queues[(int)QueueType.VideoDecode];
    public VulkanCommandQueue? VideoEncodeQueue => _queues[(int)QueueType.VideoEncode];

    public VmaAllocator Allocator => _allocator;

    public VkPipelineCache PipelineCache { get; }

    public VkBuffer NullBuffer => _nullBuffer;
    public VkImageView NullImage1DView => _nullImageView1D;
    public VkImageView NullImage2DView => _nullImageView2D;
    public VkImageView NullImage3DView => _nullImageView3D;
    public VkSampler NullSampler => _nullSampler;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanGraphicsDevice" /> class.
    /// </summary>
    ~VulkanGraphicsDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            WaitIdle();
            _shuttingDown = true;

            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].Dispose();
            }

            _copyAllocator.Dispose();

            foreach (VkSampler sampler in _samplerCache.Values)
            {
                _deviceApi.vkDestroySampler(_handle, sampler);
            }
            _samplerCache.Clear();

            // Destroy null descriptor
            vmaDestroyBuffer(_allocator, _nullBuffer, _nullBufferAllocation);
            _deviceApi.vkDestroyBufferView(_handle, _nullBufferView);
            vmaDestroyImage(_allocator, _nullImage1D, _nullImageAllocation1D);
            vmaDestroyImage(_allocator, _nullImage2D, _nullImageAllocation2D);
            vmaDestroyImage(_allocator, _nullImage3D, _nullImageAllocation3D);
            _deviceApi.vkDestroyImageView(_handle, _nullImageView1D);
            _deviceApi.vkDestroyImageView(_handle, _nullImageView1DArray);
            _deviceApi.vkDestroyImageView(_handle, _nullImageView2D);
            _deviceApi.vkDestroyImageView(_handle, _nullImageView2DArray);
            _deviceApi.vkDestroyImageView(_handle, _nullImageViewCube);
            _deviceApi.vkDestroyImageView(_handle, _nullImageViewCubeArray);
            _deviceApi.vkDestroyImageView(_handle, _nullImageView3D);

            ProcessDeletionQueue(true);
            _frameCount = 0;
            _frameIndex = 0;

            VmaTotalStatistics stats;
            vmaCalculateStatistics(_allocator, &stats);

            if (stats.total.statistics.allocationBytes > 0)
            {
                Log.Warn($"Total device memory leaked:  {stats.total.statistics.allocationBytes} bytes.");
            }

            vmaDestroyAllocator(_allocator);

            if (PipelineCache.IsNotNull)
            {
                // Destroy Vulkan pipeline cache
                _deviceApi.vkDestroyPipelineCache(Handle, PipelineCache);
            }

            if (Handle.IsNotNull)
            {
                _deviceApi.vkDestroyDevice(Handle);
            }
        }
    }

    /// <inheritdoc />
    public override CommandQueue GetCommandQueue(QueueType type) => _queues[(int)type];

    /// <inheritdoc />
    public override void WaitIdle()
    {
        VkResult result = _deviceApi.vkDeviceWaitIdle(Handle);
        if (result != VK_SUCCESS)
        {
            throw new GraphicsException("Vulkan: Failed to wait for Vulkan device idle");
        }

        ProcessDeletionQueue(true);
    }

    /// <inheritdoc />
    public override ulong CommitFrame()
    {
        // Final submits with fences
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit(_queues[i].FrameFence);
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= MaxFramesInFlight)
        {
            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _deviceApi.vkWaitForFences(_handle, _queues[i].FrameFence, true, TimeoutValue).CheckResult();
                _deviceApi.vkResetFences(_handle, _queues[i].FrameFence).CheckResult();
            }
        }

        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].FinishFrame();
        }

        ProcessDeletionQueue(false);

        return _frameCount;
    }

    public override void WriteShadingRateValue(ShadingRate rate, void* dest)
    {
        // How to compute shading rate value texel data:
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#primsrast-fragment-shading-rate-attachment

        switch (rate)
        {
            default:
            case ShadingRate.Rate1x1:
                *(byte*)dest = 0;
                break;
            case ShadingRate.Rate1x2:
                *(byte*)dest = 0x1;
                break;
            case ShadingRate.Rate2x1:
                *(byte*)dest = 0x4;
                break;
            case ShadingRate.Rate2x2:
                *(byte*)dest = 0x5;
                break;
            case ShadingRate.Rate2x4:
                *(byte*)dest = 0x6;
                break;
            case ShadingRate.Rate4x2:
                *(byte*)dest = 0x9;
                break;
            case ShadingRate.Rate4x4:
                *(byte*)dest = 0xa;
                break;
        }
    }

    public uint GetQueueFamily(QueueType queueType) => _queueFamilyIndices[(int)queueType];
    public uint GetQueueIndex(QueueType queueType) => _queueIndices[(int)queueType];

    public VulkanUploadContext Allocate(ulong size) => _copyAllocator.Allocate(size);
    public void Submit(in VulkanUploadContext context) => _copyAllocator.Submit(in context);

    public static void AddUniqueFamily(uint* sharingIndices, ref uint count, uint family)
    {
        if (family == VK_QUEUE_FAMILY_IGNORED)
            return;

        for (uint i = 0; i < count; i++)
        {
            if (sharingIndices[i] == family)
                return;
        }

        sharingIndices[count++] = family;
    }

    public void FillBufferSharingIndices(ref VkBufferCreateInfo info, uint* sharingIndices)
    {
        for (uint i = 0; i < _queueFamilyIndices.Length; i++)
        {
            AddUniqueFamily(sharingIndices, ref info.queueFamilyIndexCount, _queueFamilyIndices[i]);
        }

        if (info.queueFamilyIndexCount > 1)
        {
            // For buffers, always just use CONCURRENT access modes,
            // so we don't have to deal with acquire/release barriers in async compute.
            info.sharingMode = VkSharingMode.Concurrent;

            info.pQueueFamilyIndices = sharingIndices;
        }
        else
        {
            info.sharingMode = VkSharingMode.Exclusive;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = null;
        }
    }

    public void FillImageSharingIndices(ref VkImageCreateInfo info, uint* sharingIndices)
    {
        for (uint i = 0; i < _queueFamilyIndices.Length; i++)
        {
            AddUniqueFamily(sharingIndices, ref info.queueFamilyIndexCount, _queueFamilyIndices[i]);
        }

        if (info.queueFamilyIndexCount > 1)
        {
            // For buffers, always just use CONCURRENT access modes,
            // so we don't have to deal with acquire/release barriers in async compute.
            info.sharingMode = VkSharingMode.Concurrent;

            info.pQueueFamilyIndices = sharingIndices;
        }
        else
        {
            info.sharingMode = VkSharingMode.Exclusive;
            info.queueFamilyIndexCount = 0;
            info.pQueueFamilyIndices = null;
        }
    }

    /// <inheritdoc />
    protected override GraphicsBuffer CreateBufferCore(in BufferDescription descriptor, void* initialData)
    {
        return new VulkanBuffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescription description, TextureData* initialData)
    {
        return new VulkanTexture(this, description, initialData);
    }

    public VkSampler GetOrCreateVulkanSampler(in SamplerDescription description)
    {
        if (!_samplerCache.TryGetValue(description, out VkSampler sampler))
        {
            bool samplerMirrorClampToEdge = _adapter.Features12.samplerMirrorClampToEdge;

            // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
            VkSamplerCreateInfo createInfo = new()
            {
                flags = 0,
                pNext = null,
                magFilter = description.MagFilter.ToVk(),
                minFilter = description.MinFilter.ToVk(),
                mipmapMode = description.MipFilter.ToVk(),
                addressModeU = description.AddressModeU.ToVk(samplerMirrorClampToEdge),
                addressModeV = description.AddressModeV.ToVk(samplerMirrorClampToEdge),
                addressModeW = description.AddressModeW.ToVk(samplerMirrorClampToEdge),
                mipLodBias = 0.0f,
            };

            ushort maxAnisotropy = description.MaxAnisotropy;
            if (maxAnisotropy > 1 && _adapter.Features2.features.samplerAnisotropy == VkBool32.True)
            {
                createInfo.anisotropyEnable = true;
                createInfo.maxAnisotropy = Math.Min(maxAnisotropy, _adapter.Properties2.properties.limits.maxSamplerAnisotropy);
            }
            else
            {
                createInfo.anisotropyEnable = false;
                createInfo.maxAnisotropy = 1;
            }

            if (description.ReductionType == SamplerReductionType.Comparison)
            {
                createInfo.compareOp = description.CompareFunction.ToVk();
                createInfo.compareEnable = true;
            }
            else
            {
                createInfo.compareOp = VkCompareOp.Never;
                createInfo.compareEnable = false;
            }

            createInfo.minLod = description.LodMinClamp;
            createInfo.maxLod = (description.LodMaxClamp == float.MaxValue) ? VK_LOD_CLAMP_NONE : description.LodMaxClamp;
            createInfo.borderColor = description.BorderColor.ToVk();
            createInfo.unnormalizedCoordinates = false;

            VkSamplerReductionModeCreateInfo samplerReductionModeInfo = default;
            if (description.ReductionType == SamplerReductionType.Minimum ||
                description.ReductionType == SamplerReductionType.Maximum)
            {
                samplerReductionModeInfo = new()
                {
                    reductionMode = description.ReductionType == SamplerReductionType.Maximum ? VkSamplerReductionMode.Max : VkSamplerReductionMode.Min
                };

                createInfo.pNext = &samplerReductionModeInfo;
            }

            VkResult result = _deviceApi.vkCreateSampler(_handle, &createInfo, null, &sampler);

            if (result != VkResult.Success)
            {
                Log.Error("Vulkan: Failed to create sampler.");
                return VkSampler.Null;
            }

            _samplerCache.Add(description, sampler);
        }

        return sampler;
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescription description)
    {
        return new VulkanSampler(this, description);
    }

    /// <inheritdoc />
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescription description)
    {
        return new VulkanBindGroupLayout(this, description);
    }

    /// <inheritdoc />
    protected override BindGroup CreateBindGroupCore(BindGroupLayout layout, in BindGroupDescription description)
    {
        return new VulkanBindGroup(this, layout, description);
    }

    /// <inheritdoc />
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescription description)
    {
        return new VulkanPipelineLayout(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateRenderPipelineCore(in RenderPipelineDescription description)
    {
        return new VulkanPipeline(this, description);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        return new VulkanPipeline(this, description);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor description)
    {
        return new VulkanQueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(ISwapChainSurface surface, in SwapChainDescription descriptor)
    {
        return new VulkanSwapChain(this, surface, descriptor);
    }

    /// <inheritdoc />
    public override RenderContext BeginRenderContext(Utf8ReadOnlyString label = default)
    {
        return _queues[(int)QueueType.Graphics].BeginCommandContext(label);
    }

    public void SetObjectName(VkObjectType objectType, ulong objectHandle, string? name = default)
    {
        if (!_adapter.VkGraphicsManager.DebugUtils)
        {
            return;
        }

        _adapter.VkGraphicsManager.InstanceApi.vkSetDebugUtilsObjectNameEXT(
            _handle,
            objectType,
            objectHandle,
            name).CheckResult();
    }

    public uint GetRegisterOffset(VkDescriptorType type)
    {
        // This needs to map with ShaderCompiler
        const uint constantBuffer = 0;
        const uint shaderResource = 100;
        const uint unorderedAccess = 200;
        const uint sampler = 300;

        switch (type)
        {
            case VkDescriptorType.Sampler:
                return sampler;

            case VkDescriptorType.SampledImage:
            case VkDescriptorType.UniformTexelBuffer:
                return shaderResource;

            case VkDescriptorType.StorageImage:
            case VkDescriptorType.StorageTexelBuffer:
            case VkDescriptorType.StorageBuffer:
            case VkDescriptorType.StorageBufferDynamic:
                return unorderedAccess;

            case VkDescriptorType.UniformBuffer:
            case VkDescriptorType.UniformBufferDynamic:
                return constantBuffer;

            case VkDescriptorType.AccelerationStructureKHR:
                return shaderResource;

            default:
                ThrowHelper.ThrowInvalidOperationException();
                return 0;
        }
    }
}
