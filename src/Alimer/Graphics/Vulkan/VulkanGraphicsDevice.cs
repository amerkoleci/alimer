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
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly VkInstance _instance;
    private readonly VkInstanceApi _instanceApi;
    private readonly VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;

    private readonly uint[] _queueFamilyIndices;
    private readonly uint[] _queueIndices;
    private readonly uint[] _queueCounts;

    private readonly VkPhysicalDeviceProperties2 _properties2;
    private readonly VkPhysicalDeviceMemoryProperties2 _memoryProperties2;
    private readonly VkPhysicalDeviceFragmentShadingRateFeaturesKHR _fragmentShadingRateFeatures;
    private readonly VkPhysicalDeviceFragmentShadingRatePropertiesKHR _fragmentShadingRateProperties;
    private readonly VkPhysicalDeviceAccelerationStructureFeaturesKHR _accelerationStructureFeatures;
    private readonly VkPhysicalDeviceRayTracingPipelineFeaturesKHR _rayTracingPipelineFeatures;
    private readonly VkPhysicalDeviceRayQueryFeaturesKHR _raytracingQueryFeatures;
    private readonly VkPhysicalDeviceConditionalRenderingFeaturesEXT _conditionalRenderingFeatures;
    private readonly VkPhysicalDeviceMeshShaderFeaturesEXT _meshShaderFeatures;
    private readonly bool _textureCompressionASTC_HDR;

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

    private readonly GraphicsAdapterProperties _adapterProperties;
    private readonly GraphicsDeviceLimits _limits;
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
            vkEnumerateInstanceLayerProperties(out uint availableInstanceLayerCount).CheckResult();
            Span<VkLayerProperties> availableInstanceLayers = stackalloc VkLayerProperties[(int)availableInstanceLayerCount];
            vkEnumerateInstanceLayerProperties(availableInstanceLayers).CheckResult();

            uint extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(null, &extensionCount, null).CheckResult();
            VkExtensionProperties* availableInstanceExtensions = stackalloc VkExtensionProperties[(int)extensionCount];
            vkEnumerateInstanceExtensionProperties(null, &extensionCount, availableInstanceExtensions).CheckResult();

            UnsafeList<Utf8String> instanceExtensions = [];
            UnsafeList<Utf8String> instanceLayers = [];
            bool validationFeatures = false;

            for (int i = 0; i < extensionCount; i++)
            {
                VkUtf8String extensionName = new(availableInstanceExtensions[i].extensionName);
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
                GetOptimalValidationLayers(ref instanceLayers, availableInstanceLayers);
            }

            if (ValidationMode == ValidationMode.GPU)
            {
                vkEnumerateInstanceExtensionProperties(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME, out uint propertyCount).CheckResult();
                Span<VkExtensionProperties> availableLayerInstanceExtensions = stackalloc VkExtensionProperties[(int)propertyCount];
                vkEnumerateInstanceExtensionProperties(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME, availableLayerInstanceExtensions).CheckResult();
                for (int i = 0; i < availableLayerInstanceExtensions.Length; i++)
                {
                    fixed (byte* pExtensionName = availableLayerInstanceExtensions[i].extensionName)
                    {
                        VkUtf8String extensionName = new(pExtensionName);
                        if (extensionName == VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)
                        {
                            validationFeatures = true;
                            instanceExtensions.Add(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
                        }
                    }
                }
            }

            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = new();

            Utf8ReadOnlyString pApplicationName = Label.GetUtf8Span();
            Utf8ReadOnlyString pEngineName = "Alimer"u8;

            VkApplicationInfo appInfo = new()
            {
                pApplicationName = pApplicationName,
                applicationVersion = new VkVersion(1, 0, 0),
                pEngineName = pEngineName,
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = VkVersion.Version_1_3
            };

            using Utf8StringArray vkLayerNames = new(instanceLayers);
            using Utf8StringArray vkInstanceExtensions = new(instanceExtensions);

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
            _instanceApi = GetApi(_instance);

            if (ValidationMode != ValidationMode.Disabled && DebugUtils)
            {
                _instanceApi.vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsCreateInfo, null, out _debugMessenger).CheckResult();
            }

#if DEBUG
            Log.Info($"Created VkInstance with version: {appInfo.apiVersion.Major}.{appInfo.apiVersion.Minor}.{appInfo.apiVersion.Patch}");

            if (createInfo.enabledLayerCount > 0)
            {
                Log.Info($"Enabled {createInfo.enabledLayerCount} Validation Layers:");

                for (uint i = 0; i < createInfo.enabledLayerCount; ++i)
                {
                    string layerName = VkStringInterop.ConvertToManaged(createInfo.ppEnabledLayerNames[i])!;
                    Log.Info($"\t{layerName}");
                }
            }

            Log.Info($"Enabled {createInfo.enabledExtensionCount} Instance Extensions:");
            for (uint i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                string extensionName = VkStringInterop.ConvertToManaged(createInfo.ppEnabledExtensionNames[i])!;
                Log.Info($"\t{extensionName}");
            }
#endif
        }

        // Features
        VkPhysicalDeviceFeatures2 features2 = default;
        VkPhysicalDeviceVulkan11Features features1_1 = default;
        VkPhysicalDeviceVulkan12Features features1_2 = default;
        VkPhysicalDeviceVulkan13Features features1_3 = default;

        VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilityFeatures = default;
        VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = default;
        VkPhysicalDevicePerformanceQueryFeaturesKHR performanceQueryFeatures = default;
        VkPhysicalDeviceTextureCompressionASTCHDRFeatures astcHdrFeatures = default;

        // Core in 1.3
        VkPhysicalDeviceMaintenance4Features maintenance4Features = default;
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = default;
        VkPhysicalDeviceSynchronization2Features synchronization2Features = default;
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = default;
        VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = default;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = default;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = default;
        VkPhysicalDeviceRayQueryFeaturesKHR raytracingQueryFeatures = default;
        VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = default;
        VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = default;
        VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = default;

        // Properties
        VkPhysicalDeviceProperties2 properties2 = default;
        VkPhysicalDeviceVulkan11Properties properties1_1 = default;
        VkPhysicalDeviceVulkan12Properties properties1_2 = default;
        VkPhysicalDeviceVulkan13Properties properties1_3 = default;
        VkPhysicalDeviceMemoryProperties2 memoryProperties2 = default;

        // Core 1.2
        VkPhysicalDeviceDriverProperties driverProperties = default;
        VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = default;
        VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = default;

        VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = default;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = default;
        VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = default;
        VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = default;

        // Enumerate physical enabledDeviceExtensionsdevice and create logical device.
        {
            UnsafeList<Utf8String> enabledDeviceExtensions = [];

            _instanceApi.vkEnumeratePhysicalDevices(_instance, out uint adapterCount).CheckResult();
            if (adapterCount == 0)
            {
                throw new GraphicsException("Vulkan: Failed to find GPUs with Vulkan support");
            }

            Span<VkPhysicalDevice> physicalDevices = stackalloc VkPhysicalDevice[(int)adapterCount];
            _instanceApi.vkEnumeratePhysicalDevices(_instance, physicalDevices).CheckResult();

            foreach (VkPhysicalDevice candidatePhysicalDevice in physicalDevices)
            {
                // We require minimum 1.2
                VkPhysicalDeviceProperties physicalDeviceProperties;
                _instanceApi.vkGetPhysicalDeviceProperties(candidatePhysicalDevice, &physicalDeviceProperties);
                if (physicalDeviceProperties.apiVersion < VkVersion.Version_1_2)
                {
                    continue;
                }

                VulkanPhysicalDeviceExtensions physicalDeviceExtensions = VulkanPhysicalDeviceExtensions.Query(_instanceApi, candidatePhysicalDevice);
                if (!physicalDeviceExtensions.Swapchain)
                {
                    continue;
                }

                // Features
                void** featuresChain = null;
                features2 = new();
                features1_1 = new();
                features1_2 = new();
                features1_3 = new();

                portabilityFeatures = default;

                // Core in 1.3
                maintenance4Features = default;
                dynamicRenderingFeatures = default;
                synchronization2Features = default;
                extendedDynamicStateFeatures = default;
                extendedDynamicState2Features = default;

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

                samplerFilterMinmaxProperties = new();
                *propertiesChain = &samplerFilterMinmaxProperties;
                propertiesChain = &samplerFilterMinmaxProperties.pNext;

                depthStencilResolveProperties = new();
                *propertiesChain = &depthStencilResolveProperties;
                propertiesChain = &depthStencilResolveProperties.pNext;

                // Device extensions
                enabledDeviceExtensions =
                [
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                ];

                // Core in 1.3
                if (physicalDeviceProperties.apiVersion < VkVersion.Version_1_3)
                {
                    driverProperties = new();
                    *propertiesChain = &driverProperties;
                    propertiesChain = &driverProperties.pNext;

                    if (physicalDeviceExtensions.Maintenance4)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);

                        maintenance4Features = new();
                        *propertiesChain = &maintenance4Features;
                        propertiesChain = &maintenance4Features.pNext;
                    }

                    if (physicalDeviceExtensions.DynamicRendering)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

                        dynamicRenderingFeatures = new();
                        *propertiesChain = &dynamicRenderingFeatures;
                        propertiesChain = &dynamicRenderingFeatures.pNext;
                    }

                    if (physicalDeviceExtensions.Synchronization2)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

                        synchronization2Features = new();
                        *propertiesChain = &synchronization2Features;
                        propertiesChain = &synchronization2Features.pNext;
                    }

                    if (physicalDeviceExtensions.ExtendedDynamicState)
                    {
                        enabledDeviceExtensions.Add(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);

                        extendedDynamicStateFeatures = new();
                        *propertiesChain = &extendedDynamicStateFeatures;
                        propertiesChain = &extendedDynamicStateFeatures.pNext;
                    }

                    if (physicalDeviceExtensions.ExtendedDynamicState2)
                    {
                        enabledDeviceExtensions.Add(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);

                        extendedDynamicState2Features = new();
                        *propertiesChain = &extendedDynamicState2Features;
                        propertiesChain = &extendedDynamicState2Features.pNext;
                    }
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

                if (physicalDeviceExtensions.DepthClipEnable)
                {
                    enabledDeviceExtensions.Add(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME);

                    depthClipEnableFeatures = new();
                    *featuresChain = &depthClipEnableFeatures;
                    featuresChain = &depthClipEnableFeatures.pNext;
                }

                if (physicalDeviceExtensions.PortabilitySubset)
                {
                    enabledDeviceExtensions.Add(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);

                    portabilityFeatures = new();
                    *featuresChain = &portabilityFeatures;
                    featuresChain = &portabilityFeatures.pNext;
                }

                if (physicalDeviceExtensions.TextureCompressionAstcHdr)
                {
                    enabledDeviceExtensions.Add(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);

                    astcHdrFeatures = new();
                    *featuresChain = &astcHdrFeatures;
                    featuresChain = &astcHdrFeatures.pNext;
                }

                if (physicalDeviceExtensions.PerformanceQuery)
                {
                    enabledDeviceExtensions.Add(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);

                    performanceQueryFeatures = new();
                    *featuresChain = &performanceQueryFeatures;
                    featuresChain = &performanceQueryFeatures.pNext;
                }

                if (physicalDeviceExtensions.accelerationStructure)
                {
                    Guard.IsTrue(physicalDeviceExtensions.DeferredHostOperations);

                    // Required by VK_KHR_acceleration_structure
                    enabledDeviceExtensions.Add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);

                    enabledDeviceExtensions.Add(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

                    accelerationStructureFeatures = new();
                    *featuresChain = &accelerationStructureFeatures;
                    featuresChain = &accelerationStructureFeatures.pNext;

                    accelerationStructureProperties = new();
                    *propertiesChain = &accelerationStructureProperties;
                    propertiesChain = &accelerationStructureProperties.pNext;

                    if (physicalDeviceExtensions.raytracingPipeline)
                    {
                        // Required by VK_KHR_pipeline_library
                        enabledDeviceExtensions.Add(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
                        enabledDeviceExtensions.Add(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);

                        rayTracingPipelineFeatures = new();
                        *featuresChain = &rayTracingPipelineFeatures;
                        featuresChain = &rayTracingPipelineFeatures.pNext;

                        rayTracingPipelineProperties = new();
                        *propertiesChain = &rayTracingPipelineProperties;
                        propertiesChain = &rayTracingPipelineProperties.pNext;
                    }

                    if (physicalDeviceExtensions.rayQuery)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_RAY_QUERY_EXTENSION_NAME);

                        raytracingQueryFeatures = new();
                        *featuresChain = &raytracingQueryFeatures;
                        featuresChain = &raytracingQueryFeatures.pNext;
                    }
                }

                if (physicalDeviceExtensions.FragmentShadingRate)
                {
                    enabledDeviceExtensions.Add(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME);

                    fragmentShadingRateFeatures = new();
                    *featuresChain = &fragmentShadingRateFeatures;
                    featuresChain = &fragmentShadingRateFeatures.pNext;

                    fragmentShadingRateProperties = new();
                    *propertiesChain = &fragmentShadingRateProperties;
                    propertiesChain = &fragmentShadingRateProperties.pNext;
                }

                if (physicalDeviceExtensions.MeshShader)
                {
                    enabledDeviceExtensions.Add(VK_EXT_MESH_SHADER_EXTENSION_NAME);

                    meshShaderFeatures = new();
                    *featuresChain = &meshShaderFeatures;
                    featuresChain = &meshShaderFeatures.pNext;

                    meshShaderProperties = new();
                    *propertiesChain = &meshShaderProperties;
                    propertiesChain = &meshShaderProperties.pNext;
                }

                if (physicalDeviceExtensions.ConditionalRendering)
                {
                    enabledDeviceExtensions.Add(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME);

                    conditionalRenderingFeatures = new();
                    *featuresChain = &conditionalRenderingFeatures;
                    featuresChain = &conditionalRenderingFeatures.pNext;
                }

                if (OperatingSystem.IsWindows())
                {
                    if (physicalDeviceExtensions.ExternalMemory)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.ExternalSemaphore)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.ExternalFence)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME);
                    }
                }
                else
                {
                    if (physicalDeviceExtensions.ExternalMemory)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.ExternalSemaphore)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
                    }

                    if (physicalDeviceExtensions.ExternalFence)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME);
                    }
                }

                // Video
                if (physicalDeviceExtensions.Video.Queue)
                {
                    enabledDeviceExtensions.Add(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);

                    if (physicalDeviceExtensions.Video.DecodeQueue)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);

                        if (physicalDeviceExtensions.Video.DecodeH264)
                        {
                            enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
                        }

                        if (physicalDeviceExtensions.Video.DecodeH265)
                        {
                            enabledDeviceExtensions.Add(VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME);
                        }
                    }

#if TODO_BETA
                    if (physicalDeviceExtensions.Video.EncodeQueue)
                    {
                        enabledDeviceExtensions.Add(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);

                        if (physicalDeviceExtensions.Video.EncodeH264)
                        {
                            enabledDeviceExtensions.Add(VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME);
                        }

                        if (physicalDeviceExtensions.Video.EncodeH265)
                        {
                            enabledDeviceExtensions.Add(VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME);
                        }
                    } 
#endif
                }

                _instanceApi.vkGetPhysicalDeviceFeatures2(candidatePhysicalDevice, &features2);
                _instanceApi.vkGetPhysicalDeviceProperties2(candidatePhysicalDevice, &properties2);

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

            PhysicalDeviceExtensions = VulkanPhysicalDeviceExtensions.Query(_instanceApi, _physicalDevice);
            _instanceApi.vkGetPhysicalDeviceFeatures2(_physicalDevice, &features2);
            _instanceApi.vkGetPhysicalDeviceProperties2(_physicalDevice, &properties2);

            memoryProperties2 = new();
            _instanceApi.vkGetPhysicalDeviceMemoryProperties2(_physicalDevice, &memoryProperties2);

            if (!features2.features.textureCompressionBC &&
                !(features2.features.textureCompressionETC2 && features2.features.textureCompressionASTC_LDR))
            {
                throw new GraphicsException("Vulkan textureCompressionBC feature required or both textureCompressionETC2 and textureCompressionASTC required.");
            }

            PhysicalDeviceFeatures2 = features2;
            PhysicalDeviceFeatures1_2 = features1_2;
            PhysicalDeviceFeatures1_3 = features1_3;
            _properties2 = properties2;
            _memoryProperties2 = memoryProperties2;
            _fragmentShadingRateFeatures = fragmentShadingRateFeatures;
            _fragmentShadingRateProperties = fragmentShadingRateProperties;
            _accelerationStructureFeatures = accelerationStructureFeatures;
            _rayTracingPipelineFeatures = rayTracingPipelineFeatures;
            _raytracingQueryFeatures = raytracingQueryFeatures;
            _conditionalRenderingFeatures = conditionalRenderingFeatures;
            _meshShaderFeatures = meshShaderFeatures;
            _textureCompressionASTC_HDR = astcHdrFeatures.textureCompressionASTC_HDR;

            Guard.IsTrue(features2.features.robustBufferAccess);
            Guard.IsTrue(features2.features.depthBiasClamp);
            Guard.IsTrue(features2.features.fragmentStoresAndAtomics);
            Guard.IsTrue(features2.features.imageCubeArray);
            Guard.IsTrue(features2.features.independentBlend);
            Guard.IsTrue(features2.features.fullDrawIndexUint32);
            Guard.IsTrue(features2.features.sampleRateShading);
            Guard.IsTrue(features2.features.shaderClipDistance);
            Guard.IsTrue(features2.features.depthClamp);

            //ALIMER_VERIFY(features2.features.occlusionQueryPrecise == VK_TRUE);
            Guard.IsTrue(features1_2.descriptorIndexing);
            Guard.IsTrue(features1_2.runtimeDescriptorArray);
            Guard.IsTrue(features1_2.descriptorBindingPartiallyBound);
            Guard.IsTrue(features1_2.descriptorBindingVariableDescriptorCount);
            Guard.IsTrue(features1_2.shaderSampledImageArrayNonUniformIndexing);
            Guard.IsTrue(features1_2.timelineSemaphore);
            //Guard.IsTrue(features_1_3.synchronization2 == VK_TRUE);
            //Guard.IsTrue(features_1_3.dynamicRendering == VK_TRUE);

            DynamicRendering = features1_3.dynamicRendering || dynamicRenderingFeatures.dynamicRendering;
            Synchronization2 = features1_3.synchronization2 || synchronization2Features.synchronization2;

            DepthClipEnable = depthClipEnableFeatures.depthClipEnable;
            DepthResolveMinMax = (depthStencilResolveProperties.supportedDepthResolveModes & VkResolveModeFlags.Min) != 0 && (depthStencilResolveProperties.supportedDepthResolveModes & VkResolveModeFlags.Max) != 0;
            StencilResolveMinMax = (depthStencilResolveProperties.supportedStencilResolveModes & VkResolveModeFlags.Min) != 0 && (depthStencilResolveProperties.supportedStencilResolveModes & VkResolveModeFlags.Max) != 0;

            uint count = 0;
            _instanceApi.vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &count, null);

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

            _instanceApi.vkGetPhysicalDeviceQueueFamilyProperties2(_physicalDevice, &count, queueProps);
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
                            if (_instanceApi.vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, surface, &presentSupport) != VkResult.Success)
                                continue;
                        }
                        else
                        {
                            if (OperatingSystem.IsWindows())
                            {
                                presentSupport = _instanceApi.vkGetPhysicalDeviceWin32PresentationSupportKHR(_physicalDevice, i);
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

            VkDeviceCreateInfo createInfo = new()
            {
                pNext = &features2,
                queueCreateInfoCount = queueCreateInfosCount,
                pQueueCreateInfos = queueCreateInfos,
                enabledExtensionCount = deviceExtensionNames.Length,
                ppEnabledExtensionNames = deviceExtensionNames,
                pEnabledFeatures = null,
            };

            result = _instanceApi.vkCreateDevice(PhysicalDevice, &createInfo, null, out _handle);
            if (result != VkResult.Success)
            {
                throw new GraphicsException($"Failed to create Vulkan Logical Device, {result}");
            }

            _deviceApi = GetApi(_instance, _handle);

#if DEBUG
            Log.Info($"Enabled {createInfo.enabledExtensionCount} Device Extensions:");
            for (uint i = 0; i < createInfo.enabledExtensionCount; ++i)
            {
                string extensionName = VkStringInterop.ConvertToManaged(createInfo.ppEnabledExtensionNames[i])!;
                Log.Info($"\t{extensionName}");
            }
#endif

            // Init adapter information
            string driverDescription;
            if (properties2.properties.apiVersion >= VkVersion.Version_1_3)
            {
                driverDescription = VkStringInterop.ConvertToManaged(properties1_2.driverName)!;
                if (properties1_2.driverInfo[0] != '\0')
                {
                    driverDescription += ": " + VkStringInterop.ConvertToManaged(properties1_2.driverInfo);
                }
            }
            else
            {
                driverDescription = VkStringInterop.ConvertToManaged(driverProperties.driverName)!;
                if (driverProperties.driverInfo[0] != '\0')
                {
                    driverDescription += ": " + VkStringInterop.ConvertToManaged(driverProperties.driverInfo);
                }
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
                AdapterName = VkStringInterop.ConvertToManaged(properties2.properties.deviceName)!,
                DriverDescription = driverDescription,
                AdapterType = adapterType,
            };
        }

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

        if (PhysicalDeviceProperties.properties.apiVersion < VkVersion.Version_1_3)
        {
            if (maintenance4Features.maintenance4)
            {
                allocatorFlags |= VmaAllocatorCreateFlags.KHRMaintenance4;
            }
        }

        if (PhysicalDeviceExtensions.Maintenance5)
        {
            allocatorFlags |= VmaAllocatorCreateFlags.KHRMaintenance5;
        }

        VmaAllocatorCreateInfo allocatorCreateInfo = new()
        {
            physicalDevice = PhysicalDevice,
            device = _handle,
            instance = _instance,
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

        TimestampFrequency = (ulong)(1.0 / _properties2.properties.limits.timestampPeriod * 1000 * 1000 * 1000);

        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = _properties2.properties.limits.maxImageDimension1D,
            MaxTextureDimension2D = _properties2.properties.limits.maxImageDimension2D,
            MaxTextureDimension3D = _properties2.properties.limits.maxImageDimension3D,
            MaxTextureDimensionCube = _properties2.properties.limits.maxImageDimensionCube,
            MaxTextureArrayLayers = _properties2.properties.limits.maxImageArrayLayers,
            MaxTexelBufferDimension2D = _properties2.properties.limits.maxTexelBufferElements,

            UploadBufferTextureRowAlignment = 1,
            UploadBufferTextureSliceAlignment = 1,
            MinConstantBufferOffsetAlignment = (uint)_properties2.properties.limits.minUniformBufferOffsetAlignment,
            MaxConstantBufferBindingSize = _properties2.properties.limits.maxUniformBufferRange,
            MinStorageBufferOffsetAlignment = (uint)_properties2.properties.limits.minStorageBufferOffsetAlignment,
            MaxStorageBufferBindingSize = _properties2.properties.limits.maxStorageBufferRange,

            MaxBufferSize = ulong.MaxValue,
            MaxPushConstantsSize = _properties2.properties.limits.maxPushConstantsSize,

            MaxVertexBuffers = _properties2.properties.limits.maxVertexInputBindings,
            MaxVertexAttributes = _properties2.properties.limits.maxVertexInputAttributes,
            MaxVertexBufferArrayStride = Math.Min(_properties2.properties.limits.maxVertexInputBindingStride, _properties2.properties.limits.maxVertexInputAttributeOffset + 1),

            MaxViewports = _properties2.properties.limits.maxViewports,
            MaxColorAttachments = _properties2.properties.limits.maxColorAttachments,

            MaxComputeWorkgroupStorageSize = _properties2.properties.limits.maxComputeSharedMemorySize,
            MaxComputeInvocationsPerWorkGroup = _properties2.properties.limits.maxComputeWorkGroupInvocations,
            MaxComputeWorkGroupSizeX = _properties2.properties.limits.maxComputeWorkGroupSize[0],
            MaxComputeWorkGroupSizeY = _properties2.properties.limits.maxComputeWorkGroupSize[1],
            MaxComputeWorkGroupSizeZ = _properties2.properties.limits.maxComputeWorkGroupSize[2],

            MaxComputeWorkGroupsPerDimension = Math.Min(_properties2.properties.limits.maxComputeWorkGroupCount[0], Math.Min(_properties2.properties.limits.maxComputeWorkGroupCount[1], _properties2.properties.limits.maxComputeWorkGroupCount[2])),

            SamplerMaxAnisotropy = (ushort)PhysicalDeviceProperties.properties.limits.maxSamplerAnisotropy,
        };

        if (fragmentShadingRateFeatures.attachmentFragmentShadingRate)
        {
            _limits.VariableRateShadingTileSize = Math.Min(fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.width, fragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize.height);
        }

        if (QueryFeatureSupport(Feature.RayTracing))
        {
            _limits.RayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            _limits.RayTracingShaderTableAligment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
            _limits.RayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
            _limits.RayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
            _limits.RayTracingMaxGeometryCount = (uint)accelerationStructureProperties.maxGeometryCount;
        }
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
    public VkInstanceApi InstanceApi => _instanceApi;

    public bool SupportsD24S8 { get; }
    public bool SupportsD32S8 { get; }

    public VulkanPhysicalDeviceExtensions PhysicalDeviceExtensions { get; }

    public VkPhysicalDeviceFeatures2 PhysicalDeviceFeatures2 { get; }

    public VkPhysicalDeviceVulkan12Features PhysicalDeviceFeatures1_2 { get; }
    public VkPhysicalDeviceVulkan13Features PhysicalDeviceFeatures1_3 { get; }
    public VkPhysicalDeviceProperties2 PhysicalDeviceProperties => _properties2;
    public VkPhysicalDeviceFragmentShadingRateFeaturesKHR FragmentShadingRateFeatures => _fragmentShadingRateFeatures;
    public VkPhysicalDeviceFragmentShadingRatePropertiesKHR FragmentShadingRateProperties => _fragmentShadingRateProperties;

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

            if (_debugMessenger.IsNotNull)
            {
                _instanceApi.vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger);
            }

            _instanceApi.vkDestroyInstance(_instance);
        }
    }

    /// <inheritdoc />
    public override bool QueryFeatureSupport(Feature feature)
    {
        switch (feature)
        {
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
                createInfo.maxAnisotropy = Math.Min(maxAnisotropy, _properties2.properties.limits.maxSamplerAnisotropy);
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
        VkFormatProperties properties;
        _instanceApi.vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &properties);

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

        _instanceApi.vkSetDebugUtilsObjectNameEXT(_handle, objectType, objectHandle, name).CheckResult();
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

    [UnmanagedCallersOnly]
    private static uint DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* userData)
    {
        string message = VkStringInterop.ConvertToManaged(pCallbackData->pMessage)!;

        if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Error)
        {
            Log.Error($"[Vulkan]: {messageTypes}: {messageSeverity} - {message}");
        }
        else if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Warning)
        {
            Log.Warn($"[Vulkan]: {messageTypes}: {messageSeverity} - {message}");
        }

        Debug.WriteLine($"[Vulkan]: {messageTypes}: {messageSeverity} - {message}");

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
