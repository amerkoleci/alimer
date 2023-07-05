// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Vulkan.Vma;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe partial class VulkanGraphicsDevice : GraphicsDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly VkInstance _instance;
    private readonly VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;

    private readonly uint[] _queueFamilyIndices;
    private readonly uint[] _queueIndices;
    private readonly uint[] _queueCounts;

    private readonly VulkanPhysicalDeviceExtensions _physicalDeviceExtensions;
    private readonly VkPhysicalDevice _physicalDevice = VkPhysicalDevice.Null;
    private readonly VkDevice _handle = VkDevice.Null;
    private readonly VulkanCopyAllocator _copyAllocator;
    private readonly VkPipelineCache _pipelineCache = VkPipelineCache.Null;

    private readonly VulkanCommandQueue[] _queues = new VulkanCommandQueue[(int)QueueType.Count];
    private readonly VmaAllocator _allocator;

    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;

    public static bool IsSupported() => s_isSupported.Value;

    public VulkanGraphicsDevice(in GraphicsDeviceDescription description)
        : base(GraphicsBackendType.Vulkan, description)
    {
        Guard.IsTrue(IsSupported(), nameof(VulkanGraphicsDevice), "Vulkan is not supported");

        _queueFamilyIndices = new uint[(int)QueueType.Count];
        _queueIndices = new uint[(int)QueueType.Count];
        _queueCounts = new uint[(int)QueueType.Count];
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            _queueFamilyIndices[i] = VK_QUEUE_FAMILY_IGNORED;
        }

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
                else if (extensionName == VK_KHR_XLIB_SURFACE_EXTENSION_NAME)
                {
                    HasXlibSurface = true;
                }
                else if (extensionName == VK_KHR_XCB_SURFACE_EXTENSION_NAME)
                {
                    HasXcbSurface = true;
                }
                else if (extensionName == VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME)
                {
                    HasWaylandSurface = true;
                }
                else if (extensionName == VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME)
                {
                    HasHeadlessSurface = true;

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
            else if (OperatingSystem.IsLinux())
            {
                if (HasXlibSurface)
                {
                    instanceExtensions.Add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
                }
                else if (HasXcbSurface)
                {
                    instanceExtensions.Add(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
                }

                if (HasWaylandSurface)
                {
                    instanceExtensions.Add(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
                }
            }
            else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
            {
                instanceExtensions.Add(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
            }

            //if (HasHeadlessSurface)
            //{
            //    instanceExtensions.Add(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
            //}

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

            using VkString pApplicationName = new(Label);
            using VkString pEngineName = new("Vortice");

            VkApplicationInfo appInfo = new()
            {
                pApplicationName = pApplicationName,
                applicationVersion = new VkVersion(1, 0, 0),
                pEngineName = pEngineName,
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = VkVersion.Version_1_3
            };

            using VkStringArray vkLayerNames = new(instanceLayers);
            using VkStringArray vkInstanceExtensions = new(instanceExtensions);

            VkInstanceCreateInfo createInfo = new()
            {
                pApplicationInfo = &appInfo,
                enabledLayerCount = vkLayerNames.Length,
                ppEnabledLayerNames = vkLayerNames,
                enabledExtensionCount = vkInstanceExtensions.Length,
                ppEnabledExtensionNames = vkInstanceExtensions
            };

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
                createInfo.flags |= VkInstanceCreateFlags.EnumeratePortabilityKHR;
            }

            result = vkCreateInstance(&createInfo, null, out _instance);
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
            Log.Info($"Created VkInstance with version: {appInfo.apiVersion.Major}.{appInfo.apiVersion.Minor}.{appInfo.apiVersion.Patch}");

            if (createInfo.enabledLayerCount > 0)
            {
                Log.Info($"Enabled {createInfo.enabledLayerCount} Validation Layers:");

                //foreach (string layer in createInfo.EnabledLayerNames)
                //{
                //    Log.Info($"\t{layer}");
                //}
            }

            Log.Info($"Enabled {createInfo.enabledExtensionCount} Instance Extensions:");
            //foreach (string extensionName in createInfo.EnabledExtensionNames!)
            //{
            //    Log.Info($"\t{extensionName}");
            //}
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

            int count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &count, null);

            VkQueueFamilyProperties2* queueProps = stackalloc VkQueueFamilyProperties2[count];
            VkQueueFamilyVideoPropertiesKHR* queueFamiliesVideo = stackalloc VkQueueFamilyVideoPropertiesKHR[count];
            for (int i = 0; i < count; ++i)
            {
                queueProps[i] = new();

                if (_physicalDeviceExtensions.Video.Queue)
                {
                    queueProps[i].pNext = &queueFamiliesVideo[i];
                    queueFamiliesVideo[i] = new();
                }
            }

            vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &count, queueProps);
            int queueFamilyCount = count;

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
                            if (vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &presentSupport) != VkResult.Success)
                                continue;
                        }
                        else
                        {
                            if (OperatingSystem.IsWindows())
                            {
                                presentSupport = vkGetPhysicalDeviceWin32PresentationSupportKHR(_physicalDevice, i);
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

                        if ((videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH264EXT) == 0 &&
                            (videoCodecOperations & VkVideoCodecOperationFlagsKHR.EncodeH265EXT) == 0)
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

            if (_physicalDeviceExtensions.Video.Queue)
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

            VkDeviceQueueCreateInfo* queueCreateInfos = stackalloc VkDeviceQueueCreateInfo[queueFamilyCount];
            uint queueCreateInfosCount = 0;
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


            using VkStringArray deviceExtensionNames = new(enabledDeviceExtensions);

            VkDeviceCreateInfo deviceCreateInfo = new()
            {
                pNext = &features2,
                queueCreateInfoCount = queueCreateInfosCount,
                pQueueCreateInfos = queueCreateInfos,
                enabledExtensionCount = deviceExtensionNames.Length,
                ppEnabledExtensionNames = deviceExtensionNames,
                pEnabledFeatures = null,
            };

            result = vkCreateDevice(PhysicalDevice, &deviceCreateInfo, null, out _handle);
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
            adapterType = properties2.properties.deviceType switch
            {
                VkPhysicalDeviceType.IntegratedGpu => GpuAdapterType.IntegratedGpu,
                VkPhysicalDeviceType.DiscreteGpu => GpuAdapterType.DiscreteGpu,
                VkPhysicalDeviceType.Cpu => GpuAdapterType.Cpu,
                VkPhysicalDeviceType.VirtualGpu => GpuAdapterType.VirtualGpu,
                _ => GpuAdapterType.Other,
            };

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
        for (int i = 0; i < (int)QueueType.Count; i++)
        {
            if (_queueFamilyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
            {
                _queues[i] = new VulkanCommandQueue(this, (QueueType)i);
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

    /// <inheritdoc />
    public override GraphicsAdapterProperties AdapterInfo => _adapterProperties;

    /// <inheritdoc />
    public override GraphicsDeviceLimits Limits => _limits;

    /// <inheritdoc />
    public override ulong TimestampFrequency { get; }

    public bool DebugUtils { get; }
    public bool HasPortability { get; }
    public bool HasHeadlessSurface { get; }
    public bool HasXlibSurface { get; }
    public bool HasXcbSurface { get; }
    public bool HasWaylandSurface { get; }
    public VkInstance Instance => _instance;

    public bool SupportsExternal { get; }
    public bool SupportsD24S8 { get; }
    public bool SupportsD32S8 { get; }

    public VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2 { get; }

    public VkPhysicalDeviceVulkan12Features PhysicalDeviceFeatures1_2 { get; }
    public VkPhysicalDeviceVulkan13Features PhysicalDeviceFeatures1_3 { get; }
    public VkPhysicalDeviceProperties2 PhysicalDeviceProperties { get; }
    public bool DepthClipControl { get; }
    public VkPhysicalDevice PhysicalDevice => _physicalDevice;
    public uint GraphicsFamily => _queueFamilyIndices[(int)QueueType.Graphics];
    public uint ComputeFamily => _queueFamilyIndices[(int)QueueType.Compute];
    public uint CopyQueueFamily => _queueFamilyIndices[(int)QueueType.Copy];
    public uint VideoDecodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoDecode];
    public uint VideoEncodeQueueFamily => _queueFamilyIndices[(int)QueueType.VideoEncode];

    public VkDevice Handle => _handle;
    public VulkanCommandQueue GraphicsQueue => _queues[(int)QueueType.Graphics];
    public VulkanCommandQueue ComputeQueue => _queues[(int)QueueType.Compute];
    public VulkanCommandQueue CopyQueue => _queues[(int)QueueType.Copy];
    public VulkanCommandQueue? VideoDecodeQueue => _queues[(int)QueueType.VideoDecode];
    public VulkanCommandQueue? VideoEncodeQueue => _queues[(int)QueueType.VideoEncode];

    public VmaAllocator MemoryAllocator => _allocator;
    public VkPipelineCache PipelineCache => _pipelineCache;

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
    public override bool QueryFeatureSupport(Feature feature)
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

                vkWaitForFences(_handle, _queues[i].FrameFence, true, 0xFFFFFFFFFFFFFFFF).DebugCheckResult();
                vkResetFences(_handle, _queues[i].FrameFence).DebugCheckResult();
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
    protected override Texture CreateTextureCore(in TextureDescription descriptor, void* initialData)
    {
        return new VulkanTexture(this, descriptor, initialData);
    }

    /// <inheritdoc />
    protected override Sampler CreateSamplerCore(in SamplerDescription description)
    {
        return new VulkanSampler(this, description);
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
    protected override QueryHeap CreateQueryHeapCore(in QueryHeapDescription description)
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
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &properties);

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
        if (!DebugUtils)
        {
            return;
        }

        vkSetDebugUtilsObjectNameEXT(_handle, objectType, objectHandle, name).DebugCheckResult();
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
