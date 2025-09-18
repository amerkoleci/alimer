// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Alimer.Graphics.Vulkan.Vma;
using CommunityToolkit.Diagnostics;
using XenoAtom.Collections;
using Alimer.Utilities;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanGraphicsDevice : GraphicsDevice
{
    private readonly VulkanGraphicsAdapter _adapter;
    private readonly uint[] _queueFamilyIndices;
    private readonly uint[] _queueIndices;
    private readonly uint[] _queueCounts;

    private readonly VkPhysicalDevice _physicalDevice = VkPhysicalDevice.Null;
    private readonly VkDevice _handle = VkDevice.Null;
    private readonly VkDeviceApi _deviceApi;
    private readonly VulkanCopyAllocator _copyAllocator;
    private readonly VkPipelineCache _pipelineCache = VkPipelineCache.Null;

    private readonly VulkanCommandQueue[] _queues = new VulkanCommandQueue[(int)QueueType.Count];
    private readonly VmaAllocator _allocator;
    private readonly VmaAllocation _nullBufferAllocation = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation1D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation2D = VmaAllocation.Null;
    private readonly VmaAllocation _nullImageAllocation3D = VmaAllocation.Null;

    private readonly Dictionary<SamplerDescriptor, VkSampler> _samplerCache = [];

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

        VkResult result = VkResult.Success;

        // Enumerate physical enabledDeviceExtensionsdevice and create logical device.
        {
            UnsafeList<Utf8String> enabledDeviceExtensions = [];
            VkInstanceApi instanceApi = _adapter.VkGraphicsManager.InstanceApi;

            instanceApi.vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, out uint count);

            VkQueueFamilyProperties2* queueProps = stackalloc VkQueueFamilyProperties2[(int)count];
            VkQueueFamilyVideoPropertiesKHR* queueFamiliesVideo = stackalloc VkQueueFamilyVideoPropertiesKHR[(int)count];
            for (int i = 0; i < count; ++i)
            {
                queueProps[i] = new();

                if (PhysicalDeviceExtensions.Video.Queue)
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

            if (PhysicalDeviceExtensions.Video.Queue)
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


            using Utf8StringArray deviceExtensionNames = new(enabledDeviceExtensions);
            VkPhysicalDeviceFeatures2 features2 = _adapter.Features2;

            VkDeviceCreateInfo createInfo = new()
            {
                pNext = &features2,
                queueCreateInfoCount = queueCreateInfosCount,
                pQueueCreateInfos = queueCreateInfos,
                enabledExtensionCount = deviceExtensionNames.Length,
                ppEnabledExtensionNames = deviceExtensionNames,
                pEnabledFeatures = null,
            };

            result = _adapter.VkGraphicsManager.InstanceApi.vkCreateDevice(
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

            if (PhysicalDeviceExtensions.MemoryBudget)
            {
                allocatorFlags |= VmaAllocatorCreateFlags.EXTMemoryBudget;
            }

            if (PhysicalDeviceExtensions.AMD_DeviceCoherentMemory)
            {
                allocatorFlags |= VmaAllocatorCreateFlags.AMDDeviceCoherentMemory;
            }

            if (PhysicalDeviceFeatures1_2.bufferDeviceAddress)
            {
                allocatorFlags |= VmaAllocatorCreateFlags.BufferDeviceAddress;
            }

            if (PhysicalDeviceExtensions.MemoryPriority)
            {
                allocatorFlags |= VmaAllocatorCreateFlags.EXTMemoryPriority;
            }

            //if (PhysicalDeviceProperties.properties.apiVersion < VkVersion.Version_1_3)
            //{
            //    if (maintenance4Features.maintenance4)
            //    {
            //        allocatorFlags |= VmaAllocatorCreateFlags.KHRMaintenance4;
            //    }
            //}

            if (PhysicalDeviceExtensions.Maintenance5)
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

                    if (Synchronization2)
                    {
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
                    }
                    else
                    {
                        VkImageMemoryBarrier barrier = new()
                        {
                            oldLayout = imageInfo.initialLayout,
                            newLayout = VkImageLayout.General,
                            srcAccessMask = 0,
                            dstAccessMask = VkAccessFlags.ShaderRead | VkAccessFlags.ShaderWrite,
                            srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            image = _nullImage1D,
                            subresourceRange = new VkImageSubresourceRange(VkImageAspectFlags.Color, 0, 1, 0, 1),
                        };

                        _deviceApi.vkCmdPipelineBarrier(uploadContext.TransitionCommandBuffer,
                            VkPipelineStageFlags.Transfer, VkPipelineStageFlags.AllCommands,
                            0,
                            0, null,
                            0, null,
                            1, &barrier);

                        barrier.image = _nullImage2D;
                        barrier.subresourceRange.layerCount = 6;
                        _deviceApi.vkCmdPipelineBarrier(uploadContext.TransitionCommandBuffer,
                            VkPipelineStageFlags.Transfer, VkPipelineStageFlags.AllCommands,
                            0,
                            0, null,
                            0, null,
                            1, &barrier);

                        barrier.image = _nullImage3D;
                        barrier.subresourceRange.layerCount = 1;
                        _deviceApi.vkCmdPipelineBarrier(uploadContext.TransitionCommandBuffer,
                            VkPipelineStageFlags.Transfer, VkPipelineStageFlags.AllCommands,
                            0,
                            0, null,
                            0, null,
                            1, &barrier);
                    }

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

                _nullSampler = GetOrCreateVulkanSampler(new SamplerDescriptor());
            }

            SupportsD24S8 = IsDepthStencilFormatSupported(VkFormat.D24UnormS8Uint);
            SupportsD32S8 = IsDepthStencilFormatSupported(VkFormat.D32SfloatS8Uint);

            Debug.Assert(SupportsD24S8 || SupportsD32S8);

            //TimestampFrequency = (ulong)(1.0 / _properties2.properties.limits.timestampPeriod * 1000 * 1000 * 1000);
        }
    }

    /// <inheritdoc />
    public override GraphicsAdapter Adapter => _adapter;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public VulkanGraphicsAdapter VkAdapter => _adapter;

    public bool SupportsD24S8 { get; }
    public bool SupportsD32S8 { get; }

    public VulkanPhysicalDeviceExtensions PhysicalDeviceExtensions { get; }

    public VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2 { get; }

    public VkPhysicalDeviceVulkan12Features PhysicalDeviceFeatures1_2 { get; }
    public VkPhysicalDeviceVulkan13Features PhysicalDeviceFeatures1_3 { get; }
    //public VkPhysicalDeviceProperties2 PhysicalDeviceProperties => _properties2;
    //public VkPhysicalDeviceFragmentShadingRateFeaturesKHR FragmentShadingRateFeatures => _fragmentShadingRateFeatures;
    //public VkPhysicalDeviceFragmentShadingRatePropertiesKHR FragmentShadingRateProperties => _fragmentShadingRateProperties;

    public bool DepthClipEnable { get; }
    public bool DepthResolveMinMax { get; }
    public bool StencilResolveMinMax { get; }
    public bool DynamicRendering { get; }
    public bool Synchronization2 { get; }

    public VkPhysicalDevice PhysicalDevice => _physicalDevice;
    public uint GraphicsFamily => _queueFamilyIndices[(int)QueueType.Graphics];
    public uint ComputeFamily => _queueFamilyIndices[(int)QueueType.Compute];
    public uint CopyQueueFamily => _queueFamilyIndices[(int)QueueType.Copy];
    public uint VideoDecodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoDecode];
    public uint VideoEncodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoEncode];

    public VkDevice Handle => _handle;
    public VkDeviceApi DeviceApi => _deviceApi;
    public VkInstanceApi InstanceApi => _adapter.VkGraphicsManager.InstanceApi;
    public bool DebugUtils => _adapter.VkGraphicsManager.DebugUtils;
    public VulkanCommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public VulkanCommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public VulkanCommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public VulkanCommandQueue? VideoDecodeQueue => _queues[(int)QueueType.VideoDecode];
    public VulkanCommandQueue? VideoEncodeQueue => _queues[(int)QueueType.VideoEncode];

    public VmaAllocator Allocator => _allocator;

    public VkPipelineCache PipelineCache => _pipelineCache;

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

            _frameCount = ulong.MaxValue;
            ProcessDeletionQueue();
            _frameCount = 0;
            _frameIndex = 0;

            VmaTotalStatistics stats;
            vmaCalculateStatistics(_allocator, &stats);

            if (stats.total.statistics.allocationBytes > 0)
            {
                Log.Warn($"Total device memory leaked:  {stats.total.statistics.allocationBytes} bytes.");
            }

            vmaDestroyAllocator(_allocator);

            if (_pipelineCache.IsNotNull)
            {
                // Destroy Vulkan pipeline cache
                _deviceApi.vkDestroyPipelineCache(Handle, _pipelineCache);
            }

            if (Handle.IsNotNull)
            {
                _deviceApi.vkDestroyDevice(Handle);
            }
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
            case Feature.Depth32FloatStencil8:
                return SupportsD32S8;
#if TODO
            case Feature.TimestampQuery:
                return PhysicalDeviceProperties.properties.limits.timestampComputeAndGraphics == true;

            case Feature.PipelineStatisticsQuery:
                return PhysicalDeviceFeatures2.features.pipelineStatisticsQuery == true;

            case Feature.TextureCompressionBC:
                return PhysicalDeviceFeatures2.features.textureCompressionBC == true;

            case Feature.TextureCompressionETC2:
                return PhysicalDeviceFeatures2.features.textureCompressionETC2 == true;

            case Feature.TextureCompressionASTC:
                return PhysicalDeviceFeatures2.features.textureCompressionASTC_LDR == true;

            case Feature.TextureCompressionASTC_HDR:
                return _textureCompressionASTC_HDR;

            case Feature.IndirectFirstInstance:
                return PhysicalDeviceFeatures2.features.drawIndirectFirstInstance == true;

            case Feature.ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case Feature.RG11B10UfloatRenderable:
                _instanceApi.vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B10G11R11UfloatPack32, out VkFormatProperties rg11b10Properties);
                if ((rg11b10Properties.optimalTilingFeatures & (VkFormatFeatureFlags.ColorAttachment | VkFormatFeatureFlags.ColorAttachmentBlend)) != 0u)
                {
                    return true;
                }

                return false;

            case Feature.BGRA8UnormStorage:
                VkFormatProperties bgra8unormProperties;
                _instanceApi.vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B8G8R8A8Unorm, &bgra8unormProperties);
                if ((bgra8unormProperties.optimalTilingFeatures & VkFormatFeatureFlags.StorageImage) != 0)
                {
                    return true;
                }
                return false;

            case Feature.TessellationShader:
                return PhysicalDeviceFeatures2.features.tessellationShader == true;

            case Feature.DepthBoundsTest:
                return PhysicalDeviceFeatures2.features.depthBounds == true;

            case Feature.SamplerClampToBorder:
                return true;

            case Feature.SamplerMirrorClampToEdge:
                return PhysicalDeviceFeatures1_2.samplerMirrorClampToEdge == true;

            case Feature.SamplerMinMax:
                return PhysicalDeviceFeatures1_2.samplerFilterMinmax == true;

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

            case Feature.Predication:
                return _conditionalRenderingFeatures.conditionalRendering;

            case Feature.DescriptorIndexing:
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.runtimeDescriptorArray);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingPartiallyBound);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.descriptorBindingVariableDescriptorCount);
                //Guard.IsTrue(PhysicalDeviceFeatures1_2.shaderSampledImageArrayNonUniformIndexing);
                return PhysicalDeviceFeatures1_2.descriptorIndexing;

            case Feature.VariableRateShading:
                return _fragmentShadingRateFeatures.pipelineFragmentShadingRate;

            case Feature.VariableRateShadingTier2:
                return _fragmentShadingRateFeatures.attachmentFragmentShadingRate;

            case Feature.RayTracing:
                return PhysicalDeviceFeatures1_2.bufferDeviceAddress &&
                    _accelerationStructureFeatures.accelerationStructure &&
                    _rayTracingPipelineFeatures.rayTracingPipeline;

            case Feature.RayTracingTier2:
                return _raytracingQueryFeatures.rayQuery && QueryFeatureSupport(Feature.RayTracing);

            case Feature.MeshShader:
                return (_meshShaderFeatures.meshShader && _meshShaderFeatures.taskShader); 
#endif
        }

        return false;
    }

    /// <inheritdoc />
    public override bool QueryPixelFormatSupport(PixelFormat format)
    {
        // TODO:
        return false;
    }

#if TODO
    /// <inheritdoc />
    public override bool QueryVertexFormatSupport(VertexFormat format)
    {
        VkFormat vkFormat = format.ToVk();
        if (vkFormat == VkFormat.Undefined)
            return false;

        VkFormatProperties2 props = new();
        vkGetPhysicalDeviceFormatProperties2(PhysicalDevice, vkFormat, &props);

        // TODO:
        return false;
    } 
#endif

    /// <inheritdoc />
    public override void WaitIdle()
    {
        ThrowIfFailed(_deviceApi.vkDeviceWaitIdle(Handle));
    }

    /// <inheritdoc />
    public override void FinishFrame()
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
        if (_frameCount >= Constants.MaxFramesInFlight)
        {
            for (int i = 0; i < (int)QueueType.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _deviceApi.vkWaitForFences(_handle, _queues[i].FrameFence, true, 0xFFFFFFFFFFFFFFFF).CheckResult();
                _deviceApi.vkResetFences(_handle, _queues[i].FrameFence).CheckResult();
            }
        }

        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].FinishFrame();
        }

        ProcessDeletionQueue();
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
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, TextureData* initialData)
    {
        return new VulkanTexture(this, descriptor, initialData);
    }

    public VkSampler GetOrCreateVulkanSampler(in SamplerDescriptor description)
    {
        if (!_samplerCache.TryGetValue(description, out VkSampler sampler))
        {
            bool samplerMirrorClampToEdge = PhysicalDeviceFeatures1_2.samplerMirrorClampToEdge;

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
            if (maxAnisotropy > 1 && PhysicalDeviceFeatures2.features.samplerAnisotropy == VkBool32.True)
            {
                createInfo.anisotropyEnable = true;
                //createInfo.maxAnisotropy = Math.Min(maxAnisotropy, _properties2.properties.limits.maxSamplerAnisotropy);
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
    protected override Sampler CreateSamplerCore(in SamplerDescriptor description)
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
    public override RenderContext BeginRenderContext(string? label = null)
    {
        return _queues[(int)QueueType.Graphics].BeginCommandContext(label);
    }

    public bool IsDepthStencilFormatSupported(VkFormat format)
    {
        Debug.Assert(format == VkFormat.D16UnormS8Uint || format == VkFormat.D24UnormS8Uint || format == VkFormat.D32SfloatS8Uint || format == VkFormat.S8Uint);
        _adapter.VkGraphicsManager.InstanceApi.vkGetPhysicalDeviceFormatProperties(
            PhysicalDevice,
            format,
            out VkFormatProperties properties
            );
        return (properties.optimalTilingFeatures & VkFormatFeatureFlags.DepthStencilAttachment) != 0;
    }

    public VkFormat ToVkFormat(PixelFormat format)
    {
        //if (format == PixelFormat.Stencil8 && !SupportsS8)
        //{
        //    return VkFormat.D24UnormS8Uint;
        //}

        if (format == PixelFormat.Depth24UnormStencil8 && !SupportsD24S8)
        {
            return VkFormat.D32SfloatS8Uint;
        }

        VkFormat vkFormat = VulkanUtils.ToVkFormat(format);
        return vkFormat;
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
