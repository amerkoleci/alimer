// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.Vma;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.VkUtils;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanGraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly VkInstance _instance;
    private readonly VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;

    private readonly QueueFamilyIndices _queueFamilyIndices;
    private readonly VulkanPhysicalDeviceExtensions _physicalDeviceExtensions;
    private readonly VkPhysicalDevice _physicalDevice = VkPhysicalDevice.Null;
    private readonly VkDevice _handle = VkDevice.Null;
    private readonly VulkanCopyAllocator _copyAllocator;
    private readonly VkPipelineCache _pipelineCache = VkPipelineCache.Null;

    private readonly VulkanCommandQueue[] _queues = new VulkanCommandQueue[(int)CommandQueue.Count];
    private readonly VmaAllocator _allocator;

    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;

    public static bool IsSupported() => s_isSupported.Value;

    public VulkanGraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.Vulkan, description)
    {
        Guard.IsTrue(IsSupported(), nameof(VulkanGraphicsDevice), "Vulkan is not supported");

        VkResult result = VkResult.Success;

        // Create instance first.
        {
            int instanceLayerCount = 0;
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, null).DebugCheckResult();
            VkLayerProperties* availableInstanceLayers = stackalloc VkLayerProperties[instanceLayerCount];
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers).DebugCheckResult();

            int extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(null, &extensionCount, null).CheckResult();
            VkExtensionProperties* availableInstanceExtensions = stackalloc VkExtensionProperties[extensionCount];
            vkEnumerateInstanceExtensionProperties(null, &extensionCount, availableInstanceExtensions).CheckResult();

            List<string> instanceExtensions = new();
            List<string> instanceLayers = new();
            bool validationFeatures = false;

            for (int i = 0; i < extensionCount; i++)
            {
                string extensionName = availableInstanceExtensions[i].GetExtensionName();
                if (extensionName == VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                {
                    DebugUtils = true;
                    instanceExtensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }
                else if (extensionName == VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)
                {
                    instanceExtensions.Add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                }
                else if (extensionName == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)
                {
                    HasPortability = true;
                    instanceExtensions.Add(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                }
                else if (extensionName == VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)
                {
                    instanceExtensions.Add(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
                }
                else if (extensionName == VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)
                {
                    HasHeadlessSurface = true;
                    //instanceExtensions.Add(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
                }
            }
            instanceExtensions.Add(VK_KHR_SURFACE_EXTENSION_NAME);

            // Enable surface extensions depending on os
            if (OperatingSystem.IsAndroid())
            {
                instanceExtensions.Add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
            }
            else if (OperatingSystem.IsWindows())
            {
                instanceExtensions.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
            }
            else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
            {
                instanceExtensions.Add(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
            }
            else
            {
                instanceExtensions.Add(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
            }

            if (ValidationMode != ValidationMode.Disabled)
            {
                // Determine the optimal validation layers to enable that are necessary for useful debugging
                GetOptimalValidationLayers(availableInstanceLayers, instanceLayerCount, instanceLayers);
            }

            if (ValidationMode == ValidationMode.GPU)
            {
                ReadOnlySpan<VkExtensionProperties> availableLayerInstanceExtensions = vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation");
                for (int i = 0; i < availableLayerInstanceExtensions.Length; i++)
                {
                    string extensionName = availableLayerInstanceExtensions[i].GetExtensionName();
                    if (extensionName == VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)
                    {
                        validationFeatures = true;
                        instanceExtensions.Add(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                    }
                }
            }

            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = new();

            VkApplicationInfo appInfo = new()
            {
                ApplicationName = Label,
                ApplicationVersion = new VkVersion(1, 0, 0),
                EngineName = "Alimer",
                EngineVersion = new VkVersion(1, 0, 0),
                ApiVersion = VkVersion.Version_1_3
            };

            VkInstanceCreateInfo createInfo = new(appInfo, instanceLayers.ToArray(), instanceExtensions.ToArray());

            if (ValidationMode != ValidationMode.Disabled && DebugUtils)
            {
                debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagsEXT.Error | VkDebugUtilsMessageSeverityFlagsEXT.Warning;
                debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagsEXT.Validation | VkDebugUtilsMessageTypeFlagsEXT.Performance;

                if (ValidationMode == ValidationMode.Verbose)
                {
                    debugUtilsCreateInfo.messageSeverity |= VkDebugUtilsMessageSeverityFlagsEXT.Verbose;
                    debugUtilsCreateInfo.messageSeverity |= VkDebugUtilsMessageSeverityFlagsEXT.Info;
                }

                debugUtilsCreateInfo.pfnUserCallback = &DebugMessengerCallback;
                createInfo.pNext = &debugUtilsCreateInfo;
            }

            VkValidationFeaturesEXT validationFeaturesInfo = new();

            if (ValidationMode == ValidationMode.GPU && validationFeatures)
            {
                VkValidationFeatureEnableEXT* enabledValidationFeatures = stackalloc VkValidationFeatureEnableEXT[2]
                {
                    VkValidationFeatureEnableEXT.GpuAssistedReserveBindingSlot,
                    VkValidationFeatureEnableEXT.GpuAssisted
                };

                validationFeaturesInfo.enabledValidationFeatureCount = 2;
                validationFeaturesInfo.pEnabledValidationFeatures = enabledValidationFeatures;
                validationFeaturesInfo.pNext = createInfo.pNext;

                createInfo.pNext = &validationFeaturesInfo;
            }

            if (HasPortability)
            {
                createInfo.Flags |= VkInstanceCreateFlags.EnumeratePortabilityKHR;
            }

            result = vkCreateInstance(in createInfo, out _instance);
            if (result != VkResult.Success)
            {
                throw new InvalidOperationException($"Failed to create vulkan instance: {result}");
            }

            vkLoadInstanceOnly(_instance);

            if (ValidationMode != ValidationMode.Disabled && DebugUtils)
            {
                vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsCreateInfo, null, out _debugMessenger).CheckResult();
            }


#if DEBUG
            Log.Info($"Created VkInstance with version: {appInfo.ApiVersion.Major}.{appInfo.ApiVersion.Minor}.{appInfo.ApiVersion.Patch}");

            if (createInfo.EnabledLayerNames?.Length > 0)
            {
                Log.Info($"Enabled {createInfo.EnabledLayerNames.Length} Validation Layers:");

                foreach (string layer in createInfo.EnabledLayerNames)
                {
                    Log.Info($"\t{layer}");
                }
            }

            Log.Info($"Enabled {createInfo.EnabledExtensionNames!.Length} Instance Extensions:");
            foreach (string extensionName in createInfo.EnabledExtensionNames!)
            {
                Log.Info($"\t{extensionName}");
            }
#endif
        }


        // Enumerate physical device and create logical device.
        {
            // Features
            VkPhysicalDeviceFeatures2 features2 = default;
            VkPhysicalDeviceVulkan11Features features1_1 = default;
            VkPhysicalDeviceVulkan12Features features1_2 = default;
            VkPhysicalDeviceVulkan13Features features1_3 = default;

            VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = default;
            VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilityFeatures = default;
            VkPhysicalDevicePerformanceQueryFeaturesKHR performanceQueryFeatures = default;

            // Properties
            VkPhysicalDeviceProperties2 properties2 = default;
            VkPhysicalDeviceVulkan11Properties properties1_1 = default;
            VkPhysicalDeviceVulkan12Properties properties1_2 = default;
            VkPhysicalDeviceVulkan13Properties properties1_3 = default;

            // Core 1.2
            VkPhysicalDeviceDriverProperties driverProperties = default;
            VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = default;
            VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = default;

            bool supportsExternalSemaphore = false;
            bool supportsExternalMemory = false;
            bool video_queue = false;
            bool video_decode_queue = false;
            bool video_decode_h264 = false;
            bool video_decode_h265 = false;

            List<string> enabledDeviceExtensions = new();

            ReadOnlySpan<VkPhysicalDevice> physicalDevices = vkEnumeratePhysicalDevices(_instance);

            if (physicalDevices.Length == 0)
            {
                throw new GraphicsException("Vulkan: Failed to find GPUs with Vulkan support");
            }

            foreach (VkPhysicalDevice candidatePhysicalDevice in physicalDevices)
            {
                // We require minimum 1.2
                VkPhysicalDeviceProperties physicalDeviceProperties;
                vkGetPhysicalDeviceProperties(candidatePhysicalDevice, &physicalDeviceProperties);
                if (physicalDeviceProperties.apiVersion < VkVersion.Version_1_2)
                {
                    continue;
                }

                VulkanPhysicalDeviceExtensions physicalDeviceExtensions = QueryPhysicalDeviceExtensions(candidatePhysicalDevice);
                if (!physicalDeviceExtensions.swapchain)
                {
                    continue;
                }

                // Features
                void** featuresChain = null;
                features2 = new();
                features1_1 = new();
                features1_2 = new();
                features1_3 = new();
                depthClipEnableFeatures = new();
                portabilityFeatures = new();
                performanceQueryFeatures = new();

                features2.pNext = &features1_1;
                features1_1.pNext = &features1_2;
                if (physicalDeviceProperties.apiVersion >= VkVersion.Version_1_3)
                {
                    features1_1.pNext = &features1_2;
                    features1_2.pNext = &features1_3;
                    featuresChain = &features1_3.pNext;
                }
                else if (physicalDeviceProperties.apiVersion >= VkVersion.Version_1_2)
                {
                    features1_1.pNext = &features1_2;
                    featuresChain = &features1_2.pNext;
                }
                else
                {
                    featuresChain = &features1_1.pNext;
                }

                // Properties
                void** propertiesChain = null;
                properties2 = new();
                properties1_1 = new();
                properties1_2 = new();
                properties1_3 = new();

                properties2.pNext = &properties1_1;
                if (physicalDeviceProperties.apiVersion >= VkVersion.Version_1_3)
                {
                    properties1_1.pNext = &properties1_2;
                    properties1_2.pNext = &properties1_3;
                    propertiesChain = &properties1_3.pNext;
                }
                else if (physicalDeviceProperties.apiVersion >= VkVersion.Version_1_2)
                {
                    properties1_1.pNext = &properties1_2;
                    propertiesChain = &properties1_2.pNext;
                }
                else
                {
                    propertiesChain = &properties1_1.pNext;
                }

                driverProperties = new();
                samplerFilterMinmaxProperties = new();
                depthStencilResolveProperties = new();

                if (physicalDeviceExtensions.sampler_filter_minmax)
                {
                    *propertiesChain = &samplerFilterMinmaxProperties;
                    propertiesChain = &samplerFilterMinmaxProperties.pNext;
                }

                if (physicalDeviceExtensions.depth_stencil_resolve)
                {
                    *propertiesChain = &depthStencilResolveProperties;
                    propertiesChain = &depthStencilResolveProperties.pNext;
                }

                // Device extensions
                enabledDeviceExtensions = new()
                {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                    VK_KHR_MAINTENANCE_1_EXTENSION_NAME
                };

                // Core in 1.2
                if (physicalDeviceProperties.apiVersion < VkVersion.Version_1_2)
                {
                    if (physicalDeviceExtensions.driverProperties)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME);

                        *propertiesChain = &driverProperties;
                        propertiesChain = &driverProperties.pNext;
                    }

                    if (physicalDeviceExtensions.renderPass2)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.sampler_filter_minmax)
                    {
                        enabledDeviceExtensions.Add(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.depth_stencil_resolve)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
                    }
                }

                // Core in 1.3
                if (physicalDeviceProperties.apiVersion < VkVersion.Version_1_3)
                {
                }

                if (physicalDeviceExtensions.MemoryBudget)
                {
                    enabledDeviceExtensions.Add(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
                }

                if (physicalDeviceExtensions.AMD_DeviceCoherentMemory)
                {
                    enabledDeviceExtensions.Add(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME);
                }
                if (physicalDeviceExtensions.MemoryPriority)
                {
                    enabledDeviceExtensions.Add(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
                }

                if (physicalDeviceExtensions.depthClipEnable)
                {
                    enabledDeviceExtensions.Add(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);
                    *featuresChain = &depthClipEnableFeatures;
                    featuresChain = &depthClipEnableFeatures.pNext;
                }

                supportsExternalSemaphore = false;
                supportsExternalMemory = false;
                video_queue = false;
                video_decode_queue = false;
                video_decode_h264 = false;
                video_decode_h265 = false;

                ReadOnlySpan<VkExtensionProperties> deviceExtensions = vkEnumerateDeviceExtensionProperties(candidatePhysicalDevice);
                for (int i = 0; i < deviceExtensions.Length; ++i)
                {
                    string extensionName = deviceExtensions[i].GetExtensionName();


                    if (extensionName == VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

                        *featuresChain = &portabilityFeatures;
                        featuresChain = &portabilityFeatures.pNext;
                    }
                    else if (extensionName == VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

                        *featuresChain = &performanceQueryFeatures;
                        featuresChain = &performanceQueryFeatures.pNext;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
                        supportsExternalSemaphore = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
                        supportsExternalSemaphore = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
                        supportsExternalMemory = true;
                    }
                    else if (extensionName == VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
                        supportsExternalMemory = true;
                    }
                    else if (extensionName == VK_KHR_VIDEO_QUEUE_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
                        video_queue = true;
                    }
                    else if (extensionName == VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
                        video_decode_queue = true;
                    }
                    else if (extensionName == VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
                        video_decode_h264 = true;
                    }
                    else if (extensionName == VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
                        video_decode_h265 = true;
                    }
                }

                vkGetPhysicalDeviceFeatures2(candidatePhysicalDevice, &features2);
                vkGetPhysicalDeviceProperties2(candidatePhysicalDevice, &properties2);

                bool priority = properties2.properties.deviceType == VkPhysicalDeviceType.DiscreteGpu;
                if (description.PowerPreference == GpuPowerPreference.LowPower)
                {
                    priority = properties2.properties.deviceType == VkPhysicalDeviceType.IntegratedGpu;
                }

                if (priority || _physicalDevice.IsNull)
                {
                    _physicalDevice = candidatePhysicalDevice;
                    if (priority)
                    {
                        // If this is prioritized GPU type, look no further
                        break;
                    }
                }
            }

            if (_physicalDevice.IsNull)
            {
                throw new GraphicsException("Vulkan: Failed to find a suitable GPU");
            }

            _physicalDeviceExtensions = QueryPhysicalDeviceExtensions(_physicalDevice);
            vkGetPhysicalDeviceFeatures2(_physicalDevice, &features2);
            vkGetPhysicalDeviceProperties2(_physicalDevice, &properties2);

            PhysicalDeviceFeatures2 = features2;
            PhysicalDeviceFeatures1_2 = features1_2;
            PhysicalDeviceFeatures1_3 = features1_3;

            PhysicalDeviceProperties = properties2;
            SupportsExternal = supportsExternalSemaphore & supportsExternalMemory;

            VkPhysicalDeviceMemoryProperties2 memoryProperties2 = new();
            vkGetPhysicalDeviceMemoryProperties2(_physicalDevice, &memoryProperties2);

            Guard.IsTrue(features2.features.robustBufferAccess);
            Guard.IsTrue(features2.features.depthBiasClamp);
            Guard.IsTrue(features2.features.fragmentStoresAndAtomics);
            Guard.IsTrue(features2.features.imageCubeArray);
            Guard.IsTrue(features2.features.independentBlend);
            Guard.IsTrue(features2.features.fullDrawIndexUint32);
            Guard.IsTrue(features2.features.sampleRateShading);
            Guard.IsTrue(features2.features.shaderClipDistance);

            //ALIMER_VERIFY(features2.features.occlusionQueryPrecise == VK_TRUE);
            //Guard.IsTrue(features_1_2.descriptorIndexing == VK_TRUE);
            //Guard.IsTrue(features_1_2.runtimeDescriptorArray == VK_TRUE);
            //Guard.IsTrue(features_1_2.descriptorBindingPartiallyBound == VK_TRUE);
            //Guard.IsTrue(features_1_2.descriptorBindingVariableDescriptorCount == VK_TRUE);
            //Guard.IsTrue(features_1_2.shaderSampledImageArrayNonUniformIndexing == VK_TRUE);
            Guard.IsTrue(features1_2.timelineSemaphore);
            //Guard.IsTrue(features_1_3.synchronization2 == VK_TRUE);
            //Guard.IsTrue(features_1_3.dynamicRendering == VK_TRUE);

            if (!features2.features.textureCompressionBC &&
                !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
            {
                Log.Error("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
                return;
            }

            DepthClipControl = features2.features.depthClamp && depthClipEnableFeatures.depthClipEnable;

            _queueFamilyIndices = FindQueueFamilies(_physicalDevice, VkSurfaceKHR.Null, video_queue)!;

            List<VkDeviceQueueCreateInfo> queueCreateInfos = _queueFamilyIndices.GetQueueCreateInfos();
            VkDeviceCreateInfo deviceCreateInfo = new(queueCreateInfos.ToArray(), enabledDeviceExtensions.ToArray(), pNext: &features2);

            result = vkCreateDevice(PhysicalDevice, in deviceCreateInfo, out _handle);
            if (result != VkResult.Success)
            {
                throw new GraphicsException($"Failed to create Vulkan Logical Device, {result}");
            }

            vkLoadDevice(_handle);

            // Init adapter information
            string driverDescription;
            if (properties2.properties.apiVersion >= VkVersion.Version_1_3)
            {
                driverDescription = new string(properties1_2.driverName);
                if (properties1_2.driverInfo[0] != '\0')
                {
                    driverDescription += ": " + new string(properties1_2.driverInfo);
                }
            }
            else if (_physicalDeviceExtensions.driverProperties)
            {
                driverDescription = new string(driverProperties.driverName);
                if (driverProperties.driverInfo[0] != '\0')
                {
                    driverDescription += ": " + new string(driverProperties.driverInfo);
                }
            }
            else
            {
                driverDescription = "Vulkan driver version: " + new VkVersion(properties2.properties.driverVersion);
            }

            GpuAdapterType adapterType = GpuAdapterType.Other;
            switch (properties2.properties.deviceType)
            {
                case VkPhysicalDeviceType.IntegratedGpu:
                    adapterType = GpuAdapterType.IntegratedGpu;
                    break;
                case VkPhysicalDeviceType.DiscreteGpu:
                    adapterType = GpuAdapterType.DiscreteGpu;
                    break;
                case VkPhysicalDeviceType.Cpu:
                    adapterType = GpuAdapterType.Cpu;
                    break;

                case VkPhysicalDeviceType.VirtualGpu:
                    adapterType = GpuAdapterType.VirtualGpu;
                    break;

                default:
                    adapterType = GpuAdapterType.Other;
                    break;
            }

            _adapterProperties = new()
            {
                VendorId = properties2.properties.vendorID,
                DeviceId = properties2.properties.deviceID,
                AdapterName = new string(properties2.properties.deviceName),
                DriverDescription = driverDescription,
                AdapterType = adapterType,
            };
        }

        // Queues
        for (int queue = 0; queue < (int)CommandQueue.Count; queue++)
        {
            if (_queueFamilyIndices.FamilyIndices[queue] != VK_QUEUE_FAMILY_IGNORED)
            {
                _queues[queue] = new VulkanCommandQueue(this, (CommandQueue)queue);
            }
        }

        // Memory Allocator
        {
            VmaAllocatorCreateInfo allocatorCreateInfo;
            allocatorCreateInfo.VulkanApiVersion = VkVersion.Version_1_3;
            allocatorCreateInfo.PhysicalDevice = PhysicalDevice;
            allocatorCreateInfo.Device = Handle;
            allocatorCreateInfo.Instance = Instance;

            // Core in 1.1
            allocatorCreateInfo.flags = VmaAllocatorCreateFlags.KHRDedicatedAllocation | VmaAllocatorCreateFlags.KHRBindMemory2;

            if (_physicalDeviceExtensions.MemoryBudget)
            {
                allocatorCreateInfo.flags |= VmaAllocatorCreateFlags.ExtMemoryBudget;
            }

            if (_physicalDeviceExtensions.AMD_DeviceCoherentMemory)
            {
                allocatorCreateInfo.flags |= VmaAllocatorCreateFlags.AMDDeviceCoherentMemory;
            }

            if (PhysicalDeviceFeatures1_2.bufferDeviceAddress)
            {
                allocatorCreateInfo.flags = VmaAllocatorCreateFlags.BufferDeviceAddress;
            }

            if (_physicalDeviceExtensions.MemoryPriority)
            {
                allocatorCreateInfo.flags |= VmaAllocatorCreateFlags.ExtMemoryPriority;
            }

            vmaCreateAllocator(&allocatorCreateInfo, out _allocator).CheckResult();
        }

        _copyAllocator = new(this);

        SupportsS8 = IsDepthStencilFormatSupported(VkFormat.S8Uint);
        SupportsD24S8 = IsDepthStencilFormatSupported(VkFormat.D24UnormS8Uint);
        SupportsD32S8 = IsDepthStencilFormatSupported(VkFormat.D32SfloatS8Uint);

        Debug.Assert(SupportsD24S8 || SupportsD32S8);

        TimestampFrequency = (ulong)(1.0 / PhysicalDeviceProperties.properties.limits.timestampPeriod * 1000 * 1000 * 1000);

        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = PhysicalDeviceProperties.properties.limits.maxImageDimension1D,
            MaxTextureDimension2D = PhysicalDeviceProperties.properties.limits.maxImageDimension2D,
            MaxTextureDimension3D = PhysicalDeviceProperties.properties.limits.maxImageDimension3D,
            MaxTextureDimensionCube = PhysicalDeviceProperties.properties.limits.maxImageDimensionCube,
            MaxTextureArrayLayers = PhysicalDeviceProperties.properties.limits.maxImageArrayLayers,
            MaxTexelBufferDimension2D = PhysicalDeviceProperties.properties.limits.maxTexelBufferElements,
        };
    }

    public bool DebugUtils { get; }
    public bool HasPortability { get; }
    public bool HasHeadlessSurface { get; }
    public VkInstance Instance => _instance;

    public bool SupportsExternal { get; }
    public VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2 { get; }

    public VkPhysicalDeviceVulkan12Features PhysicalDeviceFeatures1_2 { get; }
    public VkPhysicalDeviceVulkan13Features PhysicalDeviceFeatures1_3 { get; }
    public VkPhysicalDeviceProperties2 PhysicalDeviceProperties { get; }
    public bool DepthClipControl { get; }
    public VkPhysicalDevice PhysicalDevice => _physicalDevice;
    public uint GraphicsFamily => _queueFamilyIndices.FamilyIndices[(int)CommandQueue.Graphics];
    public uint ComputeFamily => _queueFamilyIndices.FamilyIndices[(int)CommandQueue.Compute];
    public uint CopyQueueFamily => _queueFamilyIndices.FamilyIndices[(int)CommandQueue.Copy];
    public uint VideoDecodeQueueFamily => _queueFamilyIndices.FamilyIndices[(int)CommandQueue.VideoDecode];
    public uint VideoEncodeQueueFamily => _queueFamilyIndices.FamilyIndices[(int)CommandQueue.VideoEncode];

    public VkDevice Handle => _handle;
    public VulkanCommandQueue GraphicsQueue => _queues[(int)CommandQueue.Graphics];
    public VulkanCommandQueue ComputeQueue => _queues[(int)CommandQueue.Compute];
    public VulkanCommandQueue CopyQueue => _queues[(int)CommandQueue.Copy];
    public VulkanCommandQueue? VideoDecodeQueue => _queues[(int)CommandQueue.VideoDecode];
    public VulkanCommandQueue? VideoEncodeQueue => _queues[(int)CommandQueue.VideoEncode];

    public VmaAllocator MemoryAllocator => _allocator;
    public VkPipelineCache PipelineCache => _pipelineCache;

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterProperties;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public bool SupportsS8 { get; }
    public bool SupportsD24S8 { get; }
    public bool SupportsD32S8 { get; }

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

            for (int i = 0; i < (int)CommandQueue.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                _queues[i].Dispose();
            }

            _copyAllocator.Dispose();

            _frameCount = ulong.MaxValue;
            ProcessDeletionQueue();
            _frameCount = 0;
            _frameIndex = 0;

            if (_allocator.Handle != 0)
            {
                VmaTotalStatistics stats;
                vmaCalculateStatistics(_allocator, &stats);

                if (stats.total.statistics.allocationBytes > 0)
                {
                    Log.Warn($"Total device memory leaked:  {stats.total.statistics.allocationBytes} bytes.");
                }

                vmaDestroyAllocator(_allocator);
            }

            if (_pipelineCache.IsNotNull)
            {
                // Destroy Vulkan pipeline cache
                vkDestroyPipelineCache(Handle, _pipelineCache);
            }

            if (Handle.IsNotNull)
            {
                vkDestroyDevice(Handle);
            }

            if (_debugMessenger.IsNotNull)
            {
                vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger);
            }

            vkDestroyInstance(_instance);
        }
    }

    /// <inheritdoc />
    public override bool QueryFeature(Feature feature)
    {
        switch (feature)
        {
            case Feature.DepthClipControl:
                return DepthClipControl;

            case Feature.Depth32FloatStencil8:
                return SupportsD32S8;

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

            case Feature.IndirectFirstInstance:
                return PhysicalDeviceFeatures2.features.drawIndirectFirstInstance == true;

            case Feature.ShaderFloat16:
                // VK_KHR_16bit_storage core in 1.1
                // VK_KHR_shader_float16_int8 core in 1.2
                return true;

            case Feature.RG11B10UfloatRenderable:
                vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B10G11R11UfloatPack32, out VkFormatProperties rg11b10Properties);
                if ((rg11b10Properties.optimalTilingFeatures & (VkFormatFeatureFlags.ColorAttachment | VkFormatFeatureFlags.ColorAttachmentBlend)) != 0u)
                {
                    return true;
                }

                return false;

            case Feature.BGRA8UnormStorage:
                VkFormatProperties bgra8unormProperties;
                vkGetPhysicalDeviceFormatProperties(PhysicalDevice, VkFormat.B8G8R8A8Unorm, &bgra8unormProperties);
                if ((bgra8unormProperties.optimalTilingFeatures & VkFormatFeatureFlags.StorageImage) != 0)
                {
                    return true;
                }
                return false;

            case Feature.GeometryShader:
                return PhysicalDeviceFeatures2.features.geometryShader == true;

            case Feature.TessellationShader:
                return PhysicalDeviceFeatures2.features.tessellationShader == true;

            case Feature.DepthBoundsTest:
                return PhysicalDeviceFeatures2.features.depthBounds == true;

            case Feature.SamplerAnisotropy:
                return PhysicalDeviceFeatures2.features.samplerAnisotropy == true;

            case Feature.SamplerMinMax:
                return PhysicalDeviceFeatures1_2.samplerFilterMinmax == true;

                //case Feature.DepthResolveMinMax:
                //    return
                //        (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                //        (depthStencilResolveProperties.supportedDepthResolveModes & VK_RESOLVE_MODE_MAX_BIT);
                //
                //case Feature.StencilResolveMinMax:
                //    return
                //        (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MIN_BIT) &&
                //        (depthStencilResolveProperties.supportedStencilResolveModes & VK_RESOLVE_MODE_MAX_BIT);
                //
                //case Feature.ConditionalRendering:
                //    return conditionalRenderingFeatures.conditionalRendering == VK_TRUE;
        }

        return false;
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        ThrowIfFailed(vkDeviceWaitIdle(Handle));
    }

    /// <inheritdoc />
    public override void FinishFrame()
    {
        // Final submits with fences
        for (int i = 0; i < (int)CommandQueue.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].Submit(_queues[i].FrameFence);
        }

        AdvanceFrame();

        // Initiate stalling CPU when GPU is not yet finished with next frame
        if (_frameCount >= Constants.MaxFramesInFlight)
        {
            for (int i = 0; i < (int)CommandQueue.Count; i++)
            {
                if (_queues[i] is null)
                    continue;

                vkWaitForFences(_handle, _queues[i].FrameFence, true, 0xFFFFFFFFFFFFFFFF).DebugCheckResult();
                vkResetFences(_handle, _queues[i].FrameFence).DebugCheckResult();
            }
        }

        for (int i = 0; i < (int)CommandQueue.Count; i++)
        {
            if (_queues[i] is null)
                continue;

            _queues[i].FinishFrame();
        }

        ProcessDeletionQueue();
    }

    public uint GetQueueFamily(CommandQueue queueType) => _queueFamilyIndices.GetQueueFamily(queueType);
    public uint GetQueueIndex(CommandQueue queueType) => _queueFamilyIndices.GetQueueIndex(queueType);

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
        for (uint i = 0; i < _queueFamilyIndices.FamilyIndices.Length; i++)
        {
            AddUniqueFamily(sharingIndices, ref info.queueFamilyIndexCount, _queueFamilyIndices.FamilyIndices[i]);
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
        for (uint i = 0; i < _queueFamilyIndices.FamilyIndices.Length; i++)
        {
            AddUniqueFamily(sharingIndices, ref info.queueFamilyIndexCount, _queueFamilyIndices.FamilyIndices[i]);
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
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor, void* initialData)
    {
        return new VulkanTexture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Pipeline CreateComputePipelineCore(in ComputePipelineDescription description)
    {
        return new VulkanPipeline(this, description);
    }

    /// <inheritdoc />
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
    {
        return new VulkanQueryHeap(this, description);
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(SwapChainSurface surface, in SwapChainDescriptor descriptor)
    {
        return new VulkanSwapChain(this, surface, descriptor);
    }

    /// <inheritdoc />
    public override CommandBuffer BeginCommandBuffer(CommandQueue queue, string? label = null)
    {
        return _queues[(int)queue].BeginCommandBuffer(label);
    }

    public bool IsDepthStencilFormatSupported(VkFormat format)
    {
        Debug.Assert(format == VkFormat.D16UnormS8Uint || format == VkFormat.D24UnormS8Uint || format == VkFormat.D32SfloatS8Uint || format == VkFormat.S8Uint);
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &properties);

        return (properties.optimalTilingFeatures & VkFormatFeatureFlags.DepthStencilAttachment) != 0;
    }

    public VkFormat ToVkFormat(PixelFormat format)
    {
        if (format == PixelFormat.Stencil8 && !SupportsS8)
        {
            return VkFormat.D24UnormS8Uint;
        }

        if (format == PixelFormat.Depth24UnormStencil8 && !SupportsD24S8)
        {
            return VkFormat.D32SfloatS8Uint;
        }

        VkFormat vkFormat = VulkanUtils.ToVkFormat(format);
        return vkFormat;
    }

    private static QueueFamilyIndices? FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool supportsVideoQueue)
    {
        int queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, null);

        VkQueueFamilyProperties2* queueFamilies = stackalloc VkQueueFamilyProperties2[queueFamilyCount];
        VkQueueFamilyVideoPropertiesKHR* queueFamiliesVideo = stackalloc VkQueueFamilyVideoPropertiesKHR[queueFamilyCount];
        for (int i = 0; i < queueFamilyCount; ++i)
        {
            queueFamilies[i] = new();

            if (supportsVideoQueue)
            {
                queueFamilies[i].pNext = &queueFamiliesVideo[i];
                queueFamiliesVideo[i].sType = VkStructureType.QueueFamilyVideoPropertiesKHR;
            }
        }

        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilies);

        QueueFamilyIndices indices = new(queueFamilyCount);

        if (!FindVacantQueue(queueFamilyCount, CommandQueue.Graphics,
            VkQueueFlags.Graphics | VkQueueFlags.Compute, VkQueueFlags.None, 0.5f))
        {
            Log.Error("Vulkan: Could not find suitable graphics queue.");
            return default;
        }

        // Prefer another graphics queue since we can do async graphics that way.
        // The compute queue is to be treated as high priority since we also do async graphics on it.
        if (!FindVacantQueue(queueFamilyCount, CommandQueue.Compute, VkQueueFlags.Graphics | VkQueueFlags.Compute, 0, 1.0f) &&
            !FindVacantQueue(queueFamilyCount, CommandQueue.Compute, VkQueueFlags.Compute, 0, 1.0f))
        {
            // Fallback to the graphics queue if we must.
            indices.SetQueueFamily(CommandQueue.Compute, indices.GetQueueFamily(CommandQueue.Graphics));
            indices.SetQueueIndex(CommandQueue.Compute, indices.GetQueueIndex(CommandQueue.Graphics));
        }

        // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
        // If not, fallback to a dedicated compute queue.
        // Finally, fallback to same queue as compute.
        if (!FindVacantQueue(queueFamilyCount, CommandQueue.Copy, VkQueueFlags.Transfer, VkQueueFlags.Graphics | VkQueueFlags.Compute, 0.5f) &&
            !FindVacantQueue(queueFamilyCount, CommandQueue.Copy, VkQueueFlags.Compute, VkQueueFlags.Graphics, 0.5f))
        {
            indices.SetQueueFamily(CommandQueue.Copy, indices.GetQueueFamily(CommandQueue.Compute));
            indices.SetQueueIndex(CommandQueue.Copy, indices.GetQueueIndex(CommandQueue.Compute));
        }

        if (supportsVideoQueue)
        {
            if (!FindVacantQueue(queueFamilyCount, CommandQueue.VideoDecode, VkQueueFlags.VideoDecodeKHR, 0, 0.5f))
            {
                indices.SetQueueFamily(CommandQueue.VideoDecode, VK_QUEUE_FAMILY_IGNORED);
                indices.SetQueueIndex(CommandQueue.VideoDecode, uint.MaxValue);
            }

            if (!FindVacantQueue(queueFamilyCount, CommandQueue.VideoEncode, VkQueueFlags.VideoEncodeKHR, 0, 0.5f))
            {
                indices.SetQueueFamily(CommandQueue.VideoEncode, VK_QUEUE_FAMILY_IGNORED);
                indices.SetQueueIndex(CommandQueue.VideoEncode, uint.MaxValue);
            }
        }

        return indices;

        bool FindVacantQueue(int queueFamilyCount, CommandQueue queueType, VkQueueFlags required, VkQueueFlags ignoreFlags, float priority)
        {
            for (uint familyIndex = 0; familyIndex < (uint)queueFamilyCount; familyIndex++)
            {
                if ((queueFamilies[familyIndex].queueFamilyProperties.queueFlags & ignoreFlags) != 0)
                    continue;

                // A graphics queue candidate must support present for us to select it.
                if ((required & VkQueueFlags.Graphics) != 0)
                {
                    VkBool32 presentSupport = false;
                    if (surface != VkSurfaceKHR.Null)
                    {
                        if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &presentSupport) != VkResult.Success)
                            continue;
                    }
                    else
                    {
                        if (OperatingSystem.IsWindows())
                        {
                            presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, familyIndex);
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
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VkVideoCodecOperationFlagsKHR.DecodeH264) == 0 &&
                        (videoCodecOperations & VkVideoCodecOperationFlagsKHR.DecodeH265) == 0)
                    {
                        continue;
                    }
                }

                if ((required & VkQueueFlags.VideoEncodeKHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH264EXT) == 0 &&
                        (videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH265EXT) == 0)
                    {
                        continue;
                    }
                }

                if (queueFamilies[familyIndex].queueFamilyProperties.queueCount > 0 &&
                    (queueFamilies[familyIndex].queueFamilyProperties.queueFlags & required) == required)
                {
                    indices.FamilyIndices[(int)queueType] = familyIndex;
                    queueFamilies[familyIndex].queueFamilyProperties.queueCount--;
                    indices.QueueIndices[(int)queueType] = (uint)(indices.QueueOffsets[(int)familyIndex]++);
                    indices.QueuePriorities[familyIndex].Add(priority);
                    return true;
                }
            }

            return false;
        }
    }

    public void SetObjectName(VkObjectType objectType, ulong objectHandle, string? name = default)
    {
        if (!DebugUtils)
        {
            return;
        }

        vkSetDebugUtilsObjectNameEXT(_handle, objectType, objectHandle, name).DebugCheckResult();
    }

    public class QueueFamilyIndices
    {
        public int QueueFamilyCount = 0;

        public uint[] FamilyIndices;
        public uint[] QueueIndices;
        public int[] QueueCounts;

        public int[] QueueOffsets;
        public List<float>[] QueuePriorities;

        public QueueFamilyIndices(int queueFamilyCount)
        {
            FamilyIndices = new uint[(int)CommandQueue.Count];
            QueueIndices = new uint[(int)CommandQueue.Count];
            QueueCounts = new int[(int)CommandQueue.Count];
            for (int i = 0; i < (int)CommandQueue.Count; i++)
            {
                FamilyIndices[i] = VK_QUEUE_FAMILY_IGNORED;
            }

            QueueFamilyCount = queueFamilyCount;
            QueueOffsets = new int[queueFamilyCount];
            QueuePriorities = new List<float>[queueFamilyCount];

            for (int i = 0; i < queueFamilyCount; i++)
            {
                QueuePriorities[i] = new();
            }
        }

        public void SetQueueFamily(CommandQueue queueType, uint familyIndex)
        {
            FamilyIndices[(int)queueType] = familyIndex;
        }

        public void SetQueueIndex(CommandQueue queueType, uint familyIndex)
        {
            QueueIndices[(int)queueType] = familyIndex;
        }

        public uint GetQueueFamily(CommandQueue queueType) => FamilyIndices[(int)queueType];
        public uint GetQueueIndex(CommandQueue queueType) => QueueIndices[(int)queueType];

        public List<VkDeviceQueueCreateInfo> GetQueueCreateInfos()
        {
            List<VkDeviceQueueCreateInfo> result = new(QueueFamilyCount);

            for (int familyIndex = 0; familyIndex < QueueFamilyCount; familyIndex++)
            {
                if (QueueOffsets[familyIndex] == 0)
                    continue;

                VkDeviceQueueCreateInfo queueInfo = new(
                    (uint)familyIndex,
                    QueueOffsets[familyIndex],
                    QueuePriorities[familyIndex].ToArray());
                result.Add(queueInfo);
            }

            return result;
        }
    }


    [UnmanagedCallersOnly]
    private static uint DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* userData)
    {
        string message = new(pCallbackData->pMessage);
        if (messageTypes == VkDebugUtilsMessageTypeFlagsEXT.Validation)
        {
            if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Error)
            {
                Log.Error($"[Vulkan]: Validation: {messageSeverity} - {message}");
            }
            else if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Warning)
            {
                Log.Warn($"[Vulkan]: Validation: {messageSeverity} - {message}");
            }

            Debug.WriteLine($"[Vulkan]: Validation: {messageSeverity} - {message}");
        }
        else
        {
            if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Error)
            {
                Log.Error($"[Vulkan]: {messageSeverity} - {message}");
            }
            else if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Warning)
            {
                Log.Warn($"[Vulkan]: {messageSeverity} - {message}");
            }

            Debug.WriteLine($"[Vulkan]: {messageSeverity} - {message}");
        }

        return VK_FALSE;
    }

    private static bool CheckIsSupported()
    {
        try
        {
            VkResult result = vkInitialize();
            if (result != VkResult.Success)
            {
                return false;
            }

            VkVersion apiVersion = vkEnumerateInstanceVersion();
            if (apiVersion < VkVersion.Version_1_2)
            {
                Log.Warn("Vulkan 1.2 is required!");
                return false;
            }

            return true;
        }
        catch
        {
            return false;
        }
    }
}
