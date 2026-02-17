// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using XenoAtom.Collections;
using static Alimer.Graphics.Vulkan.Vma;
using static Alimer.Graphics.Vulkan.VmaMemoryUsage;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanGraphicsDevice : GraphicsDevice
{
    private const ulong TimeoutValue = 2000000000ul; // 2 seconds

    private readonly VulkanGraphicsAdapter _adapter;
    private readonly uint[] _queueFamilyIndices;
    private readonly uint[] _queueIndices;
    private readonly uint[] _queueCounts;
    private readonly GraphicsDeviceLimits _limits;

    private readonly VkPhysicalDevice _physicalDevice = VkPhysicalDevice.Null;
    private readonly VkDevice _handle = VkDevice.Null;
    private readonly VkDeviceApi _deviceApi;
    private readonly VulkanCopyAllocator _copyAllocator;
    private readonly VulkanCommandQueue[] _queues = new VulkanCommandQueue[(int)CommandQueueType.Count];
    private readonly VmaAllocator _allocator;
    private readonly uint _dynamicStateCount;
    private readonly VkDynamicState* _pDynamicStates;
    private readonly VkPipelineDynamicStateCreateInfo _dynamicStateInfo;
    private readonly VmaAllocation _nullBufferAllocation = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation1D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation2D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation3D = VmaAllocation.Null;

    private readonly Dictionary<SamplerDescriptor, VkSampler> _samplerCache = [];
    private readonly List<VkDescriptorPool> _descriptorSetPools = [];

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
        : base(GraphicsBackend.Vulkan, description)
    {
        _adapter = adapter;
        _physicalDevice = adapter.Handle;

        _queueFamilyIndices = new uint[(int)CommandQueueType.Count];
        _queueIndices = new uint[(int)CommandQueueType.Count];
        _queueCounts = new uint[(int)CommandQueueType.Count];
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
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
        float* priorities = stackalloc float[queueFamilyCount * (int)CommandQueueType.Count];
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
                    priorities[i * (int)CommandQueueType.Count + index] = priority;
                    return true;
                }
            }
            return false;
        }

        // Find graphics queue
        if (!FindVacantQueue(VkQueueFlags.Graphics | VkQueueFlags.Compute,
            VkQueueFlags.None, 0.5f,
            ref _queueFamilyIndices[(int)CommandQueueType.Graphics],
            ref _queueIndices[(int)CommandQueueType.Graphics]))
        {
            throw new GraphicsException("Vulkan: Could not find graphics queue with compute and present");
        }

        // Prefer another graphics queue since we can do async graphics that way.
        // The compute queue is to be treated as high priority since we also do async graphics on it.
        if (!FindVacantQueue(VkQueueFlags.Graphics | VkQueueFlags.Compute, VkQueueFlags.None, 1.0f, ref _queueFamilyIndices[(int)CommandQueueType.Compute], ref _queueIndices[(int)CommandQueueType.Compute]) &&
            !FindVacantQueue(VkQueueFlags.Compute, VkQueueFlags.None, 1.0f, ref _queueFamilyIndices[(int)CommandQueueType.Compute], ref _queueIndices[(int)CommandQueueType.Compute]))
        {
            _queueFamilyIndices[(int)CommandQueueType.Compute] = _queueFamilyIndices[(int)CommandQueueType.Graphics];
            _queueIndices[(int)CommandQueueType.Compute] = _queueIndices[(int)CommandQueueType.Graphics];
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(VkQueueFlags.Transfer, VkQueueFlags.Graphics | VkQueueFlags.Compute, 0.5f, ref _queueFamilyIndices[(int)CommandQueueType.Copy], ref _queueIndices[(int)CommandQueueType.Copy]) &&
            !FindVacantQueue(VkQueueFlags.Compute, VkQueueFlags.Graphics, 0.5f, ref _queueFamilyIndices[(int)CommandQueueType.Copy], ref _queueIndices[(int)CommandQueueType.Copy]))
        {
            _queueFamilyIndices[(int)CommandQueueType.Copy] = _queueFamilyIndices[(int)CommandQueueType.Compute];
            _queueIndices[(int)CommandQueueType.Copy] = _queueIndices[(int)CommandQueueType.Compute];
        }

        if (_adapter.Extensions.Video.Queue)
        {
            if (!FindVacantQueue(VkQueueFlags.VideoDecodeKHR, 0, 0.5f, ref _queueFamilyIndices[(int)CommandQueueType.VideoDecode], ref _queueIndices[(int)CommandQueueType.VideoDecode]))
            {
                _queueFamilyIndices[(int)CommandQueueType.VideoDecode] = VK_QUEUE_FAMILY_IGNORED;
                _queueIndices[(int)CommandQueueType.VideoDecode] = uint.MaxValue;
            }

            //if (!FindVacantQueue(VkQueueFlags.VideoEncodeKHR, 0, 0.5f, ref _queueFamilyIndices[(int)CommandQueueType.VideoEncode], ref _queueIndices[(int)CommandQueueType.VideoEncode]))
            //{
            //    _queueFamilyIndices[(int)CommandQueueType.VideoEncode] = VK_QUEUE_FAMILY_IGNORED;
            //    _queueIndices[(int)CommandQueueType.VideoEncode] = uint.MaxValue;
            //}
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
                pQueuePriorities = &priorities[i * (int)CommandQueueType.Count]
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

            if (_adapter.Extensions.TextureCompressionASTC_HDR)
            {
                enabledDeviceExtensions.Add(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

                var astcHdrFeatures = _adapter.ASTC_HDRFeatures;
                AddToFeatureChain(&astcHdrFeatures);
            }

            if (_adapter.Extensions.TextureCompressionASTC_3D)
            {
                enabledDeviceExtensions.Add(VK_EXT_TEXTURE_COMPRESSION_ASTC_3D_EXTENSION_NAME);

                var astc3DFeatures = _adapter.ASTC_3DFeaturesEXT;
                AddToFeatureChain(&astc3DFeatures);
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

        if (_adapter.Extensions.UnifiedImageLayouts)
        {
            enabledDeviceExtensions.Add(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME);

            VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeatures = _adapter.UnifiedImageLayoutsFeatures;
            AddToFeatureChain(&unifiedImageLayoutsFeatures);
        }

        if (_adapter.Extensions.DescriptorHeap)
        {
            enabledDeviceExtensions.Add(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);

            VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeaturesEXT = _adapter.DescriptorHeapFeaturesEXT;
            AddToFeatureChain(&descriptorHeapFeaturesEXT);
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
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            if (_queueFamilyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
            {
                _queues[i] = new VulkanCommandQueue(this, (CommandQueueType)i);
            }
        }

        _copyAllocator = new(this);

        // Dynamic PSO states
        _dynamicStateCount = 4;
        if (features2.features.depthBounds)
        {
            _dynamicStateCount++;
        }
        if (_adapter.FragmentShadingRateFeatures.pipelineFragmentShadingRate)
        {
            _dynamicStateCount++;
        }

        _pDynamicStates = MemoryUtilities.AllocateArray<VkDynamicState>(_dynamicStateCount);
        int dynamicStateIndex = 0;
        _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_VIEWPORT;
        _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_SCISSOR;
        _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
        _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        if (features2.features.depthBounds)
        {
            _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
        }
        if (_adapter.FragmentShadingRateFeatures.pipelineFragmentShadingRate)
        {
            _pDynamicStates[dynamicStateIndex++] = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
        }
        //psoDynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
        _dynamicStateInfo = new VkPipelineDynamicStateCreateInfo
        {
            dynamicStateCount = _dynamicStateCount,
            pDynamicStates = _pDynamicStates
        };

        // Pipeline Cache
        {
            // TODO: Add cache from disk
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo = new();

            _deviceApi.vkCreatePipelineCache(in pipelineCacheCreateInfo, out VkPipelineCache pipelineCache).CheckResult();
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
                format = VK_FORMAT_R32G32B32A32_SFLOAT,
                range = VK_WHOLE_SIZE,
                buffer = _nullBuffer
            };
            _deviceApi.vkCreateBufferView(in viewInfo, out _nullBufferView).CheckResult();

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

            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

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
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageView1D).CheckResult();

            imageViewInfo.image = _nullImage1D;
            imageViewInfo.viewType = VkImageViewType.Image1DArray;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageView1DArray).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.Image2D;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageView2D).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.Image2DArray;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageView2DArray).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.ImageCube;
            imageViewInfo.subresourceRange.layerCount = 6;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageViewCube).CheckResult();

            imageViewInfo.image = _nullImage2D;
            imageViewInfo.viewType = VkImageViewType.ImageCubeArray;
            imageViewInfo.subresourceRange.layerCount = 6;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageViewCubeArray).CheckResult();

            imageViewInfo.image = _nullImage3D;
            imageViewInfo.subresourceRange.layerCount = 1;
            imageViewInfo.viewType = VkImageViewType.Image3D;
            _deviceApi.vkCreateImageView(&imageViewInfo, null, out _nullImageView3D).CheckResult();

            _nullSampler = GetOrCreateVulkanSampler(new SamplerDescriptor());
        }

        // Allocate at least one descriptor pool.
        _descriptorSetPools.Add(CreateDescriptorSetPool());

        // TODO: Rest of limits
        VkPhysicalDeviceProperties2 properties2 = _adapter.Properties2;
        VkPhysicalDeviceLimits vkLimits = properties2.properties.limits;

        // VUID-VkCopyBufferToImageInfo2-dstImage-07975: If "dstImage" does not have either a depth/stencil format or a multi-planar format,
        //      "bufferOffset" must be a multiple of the texel block size
        // VUID-VkCopyBufferToImageInfo2-dstImage-07978: If "dstImage" has a depth/stencil format,
        //      "bufferOffset" must be a multiple of 4
        // Least Common Multiple stride across all formats: 1, 2, 4, 8, 16 // TODO: rarely used "12" fucks up the beauty of power-of-2 numbers, such formats must be avoided!
        const uint leastCommonMultipleStrideAccrossAllFormats = 16;

        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = vkLimits.maxImageDimension1D,
            MaxTextureDimension2D = vkLimits.maxImageDimension2D,
            MaxTextureDimension3D = vkLimits.maxImageDimension3D,
            MaxTextureDimensionCube = vkLimits.maxImageDimensionCube,
            MaxTextureArrayLayers = vkLimits.maxImageArrayLayers,
            MaxBindGroups = vkLimits.maxBoundDescriptorSets,
            MinConstantBufferOffsetAlignment = (uint)vkLimits.minUniformBufferOffsetAlignment,
            MaxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange,
            MinStorageBufferOffsetAlignment = (uint)properties2.properties.limits.minStorageBufferOffsetAlignment,
            MaxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange,

            TextureRowPitchAlignment = (uint)vkLimits.optimalBufferCopyRowPitchAlignment,
            TextureDepthPitchAlignment = MathUtilities.LeastCommonMultiple((uint)vkLimits.optimalBufferCopyOffsetAlignment, leastCommonMultipleStrideAccrossAllFormats),

            MaxBufferSize = _adapter.Properties13.maxBufferSize,
            MaxPushConstantsSize = properties2.properties.limits.maxPushConstantsSize,
            MaxColorAttachments = properties2.properties.limits.maxColorAttachments,
            MaxViewports = properties2.properties.limits.maxViewports,

            MaxVertexBuffers = properties2.properties.limits.maxVertexInputBindings,
            MaxVertexAttributes = properties2.properties.limits.maxVertexInputAttributes,
            MaxVertexBufferArrayStride = properties2.properties.limits.maxVertexInputBindingStride,

            MaxComputeWorkgroupStorageSize = properties2.properties.limits.maxComputeSharedMemorySize,
            MaxComputeInvocationsPerWorkGroup = properties2.properties.limits.maxComputeWorkGroupInvocations,
            MaxComputeWorkGroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0],
            MaxComputeWorkGroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1],
            MaxComputeWorkGroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2],
            MaxComputeWorkGroupsPerDimension = Math.Min(Math.Min(
                properties2.properties.limits.maxComputeWorkGroupCount[0],
                properties2.properties.limits.maxComputeWorkGroupCount[1]),
                properties2.properties.limits.maxComputeWorkGroupCount[2]
            )
        };

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
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public VkInstanceApi InstanceApi => _adapter.VkGraphicsManager.InstanceApi;
    public VulkanGraphicsAdapter VkAdapter => _adapter;
    public bool DebugUtils => _adapter.VkGraphicsManager.DebugUtils;

    public VkPhysicalDevice PhysicalDevice => _physicalDevice;
    public uint GraphicsFamily => _queueFamilyIndices[(int)CommandQueueType.Graphics];
    public uint ComputeFamily => _queueFamilyIndices[(int)CommandQueueType.Compute];
    public uint CopyQueueFamily => _queueFamilyIndices[(int)CommandQueueType.Copy];
    public uint VideoDecodeQueueFamily => _queueFamilyIndices[(int)CommandQueueType.VideoDecode];
    //public uint VideoEncodeQueueFamily => _queueFamilyIndices[(int)CommandQueueType.VideoEncode];

    public VkDevice Handle => _handle;
    public VkDeviceApi DeviceApi => _deviceApi;
    public VulkanCommandQueue VkGraphicsQueue => _queues[(int)CommandQueueType.Graphics];
    public VulkanCommandQueue VkComputeQueue => _queues[(int)CommandQueueType.Compute];
    public VulkanCommandQueue VkCopyQueue => _queues[(int)CommandQueueType.Copy];
    public VulkanCommandQueue? VkVideoDecodeQueue => _queues[(int)CommandQueueType.VideoDecode];
    //public VulkanCommandQueue? VideoEncodeQueue => _queues[(int)CommandQueueType.VideoEncode];

    public VmaAllocator Allocator => _allocator;

    public ref readonly VkPipelineDynamicStateCreateInfo DynamicStateInfo => ref _dynamicStateInfo;

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

            for (int i = 0; i < (int)CommandQueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].Dispose();
            }

            _copyAllocator.Dispose();

            foreach (VkSampler sampler in _samplerCache.Values)
            {
                _deviceApi.vkDestroySampler(sampler);
            }
            _samplerCache.Clear();

            // Destroy Descriptor Pools
            foreach (VkDescriptorPool descriptorPool in _descriptorSetPools)
            {
                _deviceApi.vkDestroyDescriptorPool(descriptorPool);
            }
            _descriptorSetPools.Clear();

            // Destroy null descriptor
            vmaDestroyBuffer(_allocator, _nullBuffer, _nullBufferAllocation);
            _deviceApi.vkDestroyBufferView(_nullBufferView);
            vmaDestroyImage(_allocator, _nullImage1D, _nullImageAllocation1D);
            vmaDestroyImage(_allocator, _nullImage2D, _nullImageAllocation2D);
            vmaDestroyImage(_allocator, _nullImage3D, _nullImageAllocation3D);
            _deviceApi.vkDestroyImageView(_nullImageView1D);
            _deviceApi.vkDestroyImageView(_nullImageView1DArray);
            _deviceApi.vkDestroyImageView(_nullImageView2D);
            _deviceApi.vkDestroyImageView(_nullImageView2DArray);
            _deviceApi.vkDestroyImageView(_nullImageViewCube);
            _deviceApi.vkDestroyImageView(_nullImageViewCubeArray);
            _deviceApi.vkDestroyImageView(_nullImageView3D);

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
            MemoryUtilities.Free(_pDynamicStates);

            if (PipelineCache.IsNotNull)
            {
                // Destroy Vulkan pipeline cache
                _deviceApi.vkDestroyPipelineCache(PipelineCache);
            }

            if (Handle.IsNotNull)
            {
                _deviceApi.vkDestroyDevice();
            }
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.Depth32FloatStencil8:
                return _adapter.SupportsD32S8;

            case Feature.TimestampQuery:
                return _adapter.Properties2.properties.limits.timestampComputeAndGraphics == true;

            case Feature.PipelineStatisticsQuery:
                return _adapter.Features2.features.pipelineStatisticsQuery == true;

            case Feature.TextureCompressionBC:
                return _adapter.Features2.features.textureCompressionBC == true;

            case Feature.TextureCompressionETC2:
                return _adapter.Features2.features.textureCompressionETC2 == true;

            case Feature.TextureCompressionASTC:
                return _adapter.Features2.features.textureCompressionASTC_LDR == true;

            case Feature.TextureCompressionASTC_HDR:
                return _adapter.Features13.textureCompressionASTC_HDR == true
                    || _adapter.ASTC_HDRFeatures.textureCompressionASTC_HDR == true;

            //case Feature.TextureCompressionASTC_3D:
            //    return _adapter.ASTC_3DFeaturesEXT.textureCompressionASTC_3D == true;

            case Feature.IndirectFirstInstance:
                return _adapter.Features2.features.drawIndirectFirstInstance == true;

            case Feature.ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case Feature.RG11B10UfloatRenderable:
                InstanceApi.vkGetPhysicalDeviceFormatProperties(_adapter.Handle, VkFormat.B10G11R11UfloatPack32, out VkFormatProperties rg11b10Properties);
                if ((rg11b10Properties.optimalTilingFeatures & (VkFormatFeatureFlags.ColorAttachment | VkFormatFeatureFlags.ColorAttachmentBlend)) != 0u)
                {
                    return true;
                }

                return false;

            case Feature.BGRA8UnormStorage:
                VkFormatProperties bgra8unormProperties;
                InstanceApi.vkGetPhysicalDeviceFormatProperties(_adapter.Handle, VkFormat.B8G8R8A8Unorm, &bgra8unormProperties);
                if ((bgra8unormProperties.optimalTilingFeatures & VkFormatFeatureFlags.StorageImage) != 0)
                {
                    return true;
                }
                return false;

            case Feature.TextureComponentSwizzle:
                return true;

            case Feature.DepthBoundsTest:
                return _adapter.Features2.features.depthBounds;

            case Feature.SamplerClampToBorder:
                return true;

            case Feature.SamplerMirrorClampToEdge:
                return _adapter.Features12.samplerMirrorClampToEdge;

            case Feature.SamplerMinMax:
                return _adapter.Features12.samplerFilterMinmax;

#if TODO
            case Feature.DepthResolveMinMax:
                return DepthResolveMinMax;

            case Feature.StencilResolveMinMax:
                return StencilResolveMinMax;

            case Feature.CacheCoherentUMA:
                if (_memoryProperties2.memoryProperties.memoryHeapCount == 1u &&
                    _memoryProperties2.memoryProperties.memoryHeaps[0].flags.HasFlag(VkMemoryHeapFlags.DeviceLocal))
                {
                    return true;
                }

                return false;

#endif
            case Feature.Predication:
                return _adapter.ConditionalRenderingFeatures.conditionalRendering;

            case Feature.DescriptorIndexing:
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.runtimeDescriptorArray);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingPartiallyBound);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingVariableDescriptorCount);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.shaderSampledImageArrayNonUniformIndexing);
                return _adapter.Features12.descriptorIndexing;

            case Feature.VariableRateShading:
                return _adapter.FragmentShadingRateFeatures.pipelineFragmentShadingRate;

            case Feature.VariableRateShadingTier2:
                return _adapter.FragmentShadingRateFeatures.attachmentFragmentShadingRate;

            case Feature.RayTracing:
                return _adapter.Features12.bufferDeviceAddress
                    && _adapter.AccelerationStructureFeatures.accelerationStructure
                    && _adapter.RayTracingPipelineFeatures.rayTracingPipeline;

            case Feature.RayTracingTier2:
                return _adapter.RayQueryFeatures.rayQuery && QueryFeatureSupport(Feature.RayTracing);

            case Feature.MeshShader:
                return _adapter.MeshShaderFeatures.meshShader && _adapter.MeshShaderFeatures.taskShader;

            default:
                return false;
        }
    }

    /// <inheritdoc />
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format)
    {
        VkFormat vkFormat = ToVkFormat(format);
        if (vkFormat == VK_FORMAT_UNDEFINED)
            return PixelFormatSupport.None;

        VkFormatProperties2 props = new();
        InstanceApi.vkGetPhysicalDeviceFormatProperties2(_adapter.Handle, vkFormat, &props);

        VkFormatFeatureFlags transferBits = VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;

        PixelFormatSupport result = PixelFormatSupport.None;
        if ((props.formatProperties.optimalTilingFeatures & transferBits) != 0)
            result |= PixelFormatSupport.Texture;

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
            result |= PixelFormatSupport.DepthStencil;

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0)
            result |= PixelFormatSupport.RenderTarget;

        if ((props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT) != 0)
            result |= PixelFormatSupport.Blendable;

        if (props.formatProperties.optimalTilingFeatures.HasFlag(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
            || props.formatProperties.bufferFeatures.HasFlag(VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
        {
            result |= PixelFormatSupport.ShaderLoad;
        }

        if (props.formatProperties.optimalTilingFeatures.HasFlag(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
            result |= PixelFormatSupport.ShaderSample;

        if (props.formatProperties.optimalTilingFeatures.HasFlag(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            || props.formatProperties.bufferFeatures.HasFlag(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT))
        {
            result |= PixelFormatSupport.ShaderUavLoad;
            result |= PixelFormatSupport.ShaderUavStore;
        }

        if (props.formatProperties.optimalTilingFeatures.HasFlag(VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT)
            || props.formatProperties.bufferFeatures.HasFlag(VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT))
        {
            result |= PixelFormatSupport.ShaderAtomic;
        }

#if TODO
        // Ensure that the handle type is supported.
        VkImageFormatProperties2 props2 = { };
        props2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_1D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture1D/Texture1DArray
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture2D/Texture2DArray
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_3D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, 0, nullptr, &props2))
        {
            // Texture3D
        }

        if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, nullptr, &props2))
        {
            // TextureCube/TextureCubeArray
        }

        props2 = new();
        TextureSampleCount supportedSampleCount = TextureSampleCount.Count1;
        if (format.IsDepthStencilFormat())
        {
            if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, nullptr, &props2))
            {
                supportedSampleCount = static_cast<TextureSampleCount>(props2.imageFormatProperties.sampleCounts);
            }
        }
        else
        {
            if (GetImageFormatProperties(vkFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT, 0, nullptr, &props2))
            {
                supportedSampleCount = static_cast<TextureSampleCount>(props2.imageFormatProperties.sampleCounts);
            }
        } 
#endif

        return result;
    }

    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexAttributeFormat format)
    {
        VkFormat vkFormat = format.ToVk();
        if (vkFormat == VK_FORMAT_UNDEFINED)
            return false;

        VkFormatProperties3 props3 = new();
        VkFormatProperties2 props2 = new()
        {
            pNext = &props3
        };
        InstanceApi.vkGetPhysicalDeviceFormatProperties2(_adapter.Handle, vkFormat, &props2);

        if ((props3.bufferFeatures & (VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT)) == (VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT))
            return true;

        return false;
    }

    public VkFormat ToVkFormat(PixelFormat format) => _adapter.ToVkFormat(format);

    /// <inheritdoc />
    public override CommandQueue GetCommandQueue(CommandQueueType type) => _queues[(int)type];

    /// <inheritdoc />
    public override void WaitIdle()
    {
        VkResult result = _deviceApi.vkDeviceWaitIdle();
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
        for (int i = 0; i < (int)CommandQueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit(_queues[i].FrameFence);
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= MaxFramesInFlight)
        {
            for (int i = 0; i < (int)CommandQueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _deviceApi.vkWaitForFences(_queues[i].FrameFence, true, TimeoutValue).CheckResult();
                _deviceApi.vkResetFences(_queues[i].FrameFence).CheckResult();
            }
        }

        for (int i = 0; i < (int)CommandQueueType.Count; i++)
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

    public uint GetQueueFamily(CommandQueueType queueType) => _queueFamilyIndices[(int)queueType];
    public uint GetQueueIndex(CommandQueueType queueType) => _queueIndices[(int)queueType];

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
    protected override GraphicsBuffer CreateBufferCore(in BufferDescriptor descriptor, void* initialData)
    {
        return new VulkanBuffer(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData)
    {
        return new VulkanTexture(this, descriptor, initialData);
    }

    public VkSampler GetOrCreateVulkanSampler(in SamplerDescriptor descriptor)
    {
        if (!_samplerCache.TryGetValue(descriptor, out VkSampler sampler))
        {
            bool samplerMirrorClampToEdge = _adapter.Features12.samplerMirrorClampToEdge;

            // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkSamplerCreateInfo.html
            VkSamplerCreateInfo createInfo = new()
            {
                flags = 0,
                pNext = null,
                magFilter = descriptor.MagFilter.ToVk(),
                minFilter = descriptor.MinFilter.ToVk(),
                mipmapMode = descriptor.MipFilter.ToVk(),
                addressModeU = descriptor.AddressModeU.ToVk(samplerMirrorClampToEdge),
                addressModeV = descriptor.AddressModeV.ToVk(samplerMirrorClampToEdge),
                addressModeW = descriptor.AddressModeW.ToVk(samplerMirrorClampToEdge),
                mipLodBias = 0.0f,
            };

            ushort maxAnisotropy = descriptor.MaxAnisotropy;
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

            if (descriptor.ReductionType == SamplerReductionType.Comparison)
            {
                createInfo.compareOp = descriptor.CompareFunction.ToVk();
                createInfo.compareEnable = true;
            }
            else
            {
                createInfo.compareOp = VkCompareOp.Never;
                createInfo.compareEnable = false;
            }

            createInfo.minLod = descriptor.LodMinClamp;
            createInfo.maxLod = (descriptor.LodMaxClamp == float.MaxValue) ? VK_LOD_CLAMP_NONE : descriptor.LodMaxClamp;
            createInfo.borderColor = descriptor.BorderColor.ToVk();
            createInfo.unnormalizedCoordinates = false;

            VkSamplerReductionModeCreateInfo samplerReductionModeInfo = default;
            if (descriptor.ReductionType == SamplerReductionType.Minimum ||
                descriptor.ReductionType == SamplerReductionType.Maximum)
            {
                samplerReductionModeInfo = new()
                {
                    reductionMode = descriptor.ReductionType == SamplerReductionType.Maximum ? VkSamplerReductionMode.Max : VkSamplerReductionMode.Min
                };

                createInfo.pNext = &samplerReductionModeInfo;
            }

            VkResult result = _deviceApi.vkCreateSampler(in createInfo, out sampler);

            if (result != VK_SUCCESS)
            {
                Log.Error("Vulkan: Failed to create sampler.");
                return VkSampler.Null;
            }

            _samplerCache.Add(descriptor, sampler);
        }

        return sampler;
    }

    #region DescriptorPool
    public void AllocateDescriptorPool()
    {
        _descriptorSetPools.Add(CreateDescriptorSetPool());
    }

    public VkResult AllocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorSet* descriptorSet,
        uint maxVariableDescriptorCounts)
    {
        VkDescriptorPool descriptorPool = _descriptorSetPools[^1];

        VkDescriptorSetAllocateInfo allocInfo = new()
        {
            sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            descriptorPool = descriptorPool,
            descriptorSetCount = 1,
            pSetLayouts = &descriptorSetLayout
        };

        // For variable length descriptor arrays, this specify the maximum count we expect them to be.
        // Note that this value will apply to all bindings defined as variable arrays in the BindGroupLayout
        // used to allocate this BindGroup
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableLengthInfo = new();
        if (maxVariableDescriptorCounts > 0)
        {
            variableLengthInfo.descriptorSetCount = 1;
            variableLengthInfo.pDescriptorCounts = &maxVariableDescriptorCounts;
            allocInfo.pNext = &variableLengthInfo;
        }

        return _deviceApi.vkAllocateDescriptorSets(&allocInfo, descriptorSet);
    }

    private VkDescriptorPool CreateDescriptorSetPool()
    {
        uint totalSets = 1024;
        const uint poolSizeCount = 8; // 7 + 1 raytracing

        VkDescriptorPoolSize* poolSizes = stackalloc VkDescriptorPoolSize[(int)poolSizeCount]
        {
            new(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 512),
            new(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 16 ),
            new(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512 ),
            new(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128 ),
            new(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 128 ),
            new(VK_DESCRIPTOR_TYPE_SAMPLER, 16 ), /* Static samplers are 10 */
            new(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 8 ),
            new (VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 8) // // DESCRIPTORBINDER_SRV_COUNT * poolSize
        };

        uint actualPoolSizeCount = 7;
        if (QueryFeatureSupport(Feature.RayTracing))
        {
            actualPoolSizeCount++;
        }

        VkDescriptorPoolCreateInfo poolInfo = new()
        {
            sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            maxSets = totalSets,
            flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, // VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT (bindless)
            poolSizeCount = actualPoolSizeCount,
            pPoolSizes = poolSizes
        };

        VkResult result = _deviceApi.vkCreateDescriptorPool(&poolInfo, null, out VkDescriptorPool pool);
        if (result != VK_SUCCESS)
        {
            //Log.Error(result, "Error when creating descriptor pool: {}");
            return VkDescriptorPool.Null;
        }

        return pool;
    }
    #endregion

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescriptor descriptor)
    {
        return new VulkanSampler(this, descriptor);
    }

    /// <inheritdoc />
    protected override BindGroupLayout CreateBindGroupLayoutCore(in BindGroupLayoutDescriptor description)
    {
        return new VulkanBindGroupLayout(this, description);
    }

    /// <inheritdoc />
    protected override PipelineLayout CreatePipelineLayoutCore(in PipelineLayoutDescriptor description)
    {
        return new VulkanPipelineLayout(this, description);
    }

    /// <inheritdoc />
    protected override ShaderModule CreateShaderModuleCore(in ShaderModuleDescriptor descriptor)
    {
        return new VulkanShaderModule(this, descriptor);
    }

    /// <inheritdoc />
    protected override RenderPipeline CreateRenderPipelineCore(in RenderPipelineDescriptor descriptor)
    {
        return new VulkanRenderPipeline(this, descriptor);
    }

    /// <inheritdoc />
    protected override ComputePipeline CreateComputePipelineCore(in ComputePipelineDescriptor description)
    {
        return new VulkanComputePipeline(this, description);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescriptor descriptor)
    {
        return new VulkanQueryHeap(this, in descriptor);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(in SwapChainDescriptor descriptor)
    {
        return new VulkanSwapChain(this, descriptor);
    }

    /// <inheritdoc />
    public override CommandBuffer AcquireCommandBuffer(CommandQueueType queue, Utf8ReadOnlyString label = default)
    {
        return _queues[(int)queue].AcquireCommandBuffer(label);
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

    public (VkDescriptorType DescriptorType, uint RegisterOffet) GetVkDescriptorType(BindGroupLayoutEntry entry)
    {
        bool readOnlyStorage = false;
        VkDescriptorType vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        switch (entry.BindingType)
        {
            case BindingInfoType.Buffer:
                switch (entry.Buffer.Type)
                {
                    case BufferBindingType.Constant:
                        if (entry.Buffer.HasDynamicOffset)
                        {
                            vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                        }
                        else
                        {
                            vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        }
                        break;

                    case BufferBindingType.ShaderRead:
                    case BufferBindingType.ShaderReadWrite:
                        // UniformTexelBuffer, StorageTexelBuffer ?
                        readOnlyStorage = (entry.Buffer.Type == BufferBindingType.ShaderRead);
                        if (entry.Buffer.HasDynamicOffset)
                        {
                            vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        }
                        else
                        {
                            vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        }
                        break;
                }
                break;

            case BindingInfoType.Sampler:
                vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                break;

            case BindingInfoType.Texture:
                vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;

            case BindingInfoType.StorageTexture:
                vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;

            case BindingInfoType.AccelerationStructure:
                vkDescriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                break;

            default:
                ThrowHelper.ThrowInvalidOperationException();
                break;
        }

        uint registerOffset = GetRegisterOffset(vkDescriptorType, readOnlyStorage);

        return (vkDescriptorType, registerOffset);
    }

    public uint GetRegisterOffset(VkDescriptorType type, bool readOnlyStorage = false)
    {
        // This needs to map with ShaderCompiler
        switch (type)
        {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                return VulkanRegisterShift.ContantBuffer;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                return VulkanRegisterShift.SRV;

            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                return readOnlyStorage ? VulkanRegisterShift.SRV : VulkanRegisterShift.UAV;

            case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                return VulkanRegisterShift.SRV;

            case VK_DESCRIPTOR_TYPE_SAMPLER:
                return VulkanRegisterShift.Sampler;

            default:
                ThrowHelper.ThrowInvalidOperationException();
                return 0;
        }
    }
}
