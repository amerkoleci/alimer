// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics;

public sealed unsafe class VulkanGraphicsDevice : GraphicsDevice
{
    private static readonly VkString s_appName = new("Vortice");
    private static readonly VkString s_engineName = new("Vortice");

    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

#if !NET5_0_OR_GREATER
    private readonly PFN_vkDebugUtilsMessengerCallbackEXT DebugMessagerCallbackDelegate = DebugMessengerCallback;
#endif

    private readonly VkInstance _instance;
    private readonly VkDevice _handle;
    private readonly GraphicsDeviceCaps _caps;

    public VulkanGraphicsDevice(ValidationMode validationMode = ValidationMode.Disabled, GpuPowerPreference powerPreference = GpuPowerPreference.HighPerformance)
        : base(GpuBackend.Vulkan)
    {
        if (!s_isSupported.Value)
        {
            throw new InvalidOperationException("Vulkan is not supported");
        }

        // Create instance and debug utils
        {
            ReadOnlySpan<VkExtensionProperties> availableInstanceExtensions = vkEnumerateInstanceExtensionProperties();
            ReadOnlySpan<VkLayerProperties> availableInstanceLayers = vkEnumerateInstanceLayerProperties();

            List<string> instanceExtensions = new();
            List<string> instanceLayers = new();

            instanceExtensions.Add(VK_KHR_SURFACE_EXTENSION_NAME);

#if NET5_0_OR_GREATER
            if (OperatingSystem.IsWindows())
            {
                instanceExtensions.Add("VK_KHR_win32_surface");
            }
            else if (OperatingSystem.IsAndroid())
            {
                instanceExtensions.Add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
            }
            else if (OperatingSystem.IsIOS())
            {
                instanceExtensions.Add(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
            }
            else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
            {
                instanceExtensions.Add(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
            }
            else if (OperatingSystem.IsLinux())
            {
                instanceExtensions.Add("VK_KHR_xcb_surface");
            }
#else
            // TODO
#endif

            for (int i = 0; i < availableInstanceExtensions.Length; i++)
            {
                if (availableInstanceExtensions[i].GetExtensionName() == EXTDebugUtilsExtensionName)
                {
                    instanceExtensions.Add(EXTDebugUtilsExtensionName);
                    DebugUtils = true;
                }
                else if (availableInstanceExtensions[i].GetExtensionName() == KHRGetPhysicalDeviceProperties2ExtensionName)
                {
                    instanceExtensions.Add(KHRGetPhysicalDeviceProperties2ExtensionName);
                }
                else if (availableInstanceExtensions[i].GetExtensionName() == EXTSwapchainColorSpaceExtensionName)
                {
                    instanceExtensions.Add(EXTSwapchainColorSpaceExtensionName);
                }
            }

            if (validationMode != ValidationMode.Disabled)
            {
                // Determine the optimal validation layers to enable that are necessary for useful debugging
                instanceLayers.Add("VK_LAYER_KHRONOS_validation");
            }

            VkApplicationInfo appInfo = new()
            {
                sType = VkStructureType.ApplicationInfo,
                pApplicationName = s_appName,
                applicationVersion = new VkVersion(1, 0, 0),
                pEngineName = s_engineName,
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = VkVersion.Version_1_2
            };

            using var vkLayerNames = new VkStringArray(instanceLayers);
            using VkStringArray vkInstanceExtensions = new(instanceExtensions);

            VkInstanceCreateInfo createInfo = new()
            {
                sType = VkStructureType.InstanceCreateInfo,
                pApplicationInfo = &appInfo,
                enabledLayerCount = vkLayerNames.Length,
                ppEnabledLayerNames = vkLayerNames,
                enabledExtensionCount = vkInstanceExtensions.Length,
                ppEnabledExtensionNames = vkInstanceExtensions
            };

            VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = new()
            {
                sType = VkStructureType.DebugUtilsMessengerCreateInfoEXT
            };

            if (validationMode != ValidationMode.Disabled && DebugUtils)
            {
                debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagsEXT.Error | VkDebugUtilsMessageSeverityFlagsEXT.Warning;
                debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagsEXT.Validation | VkDebugUtilsMessageTypeFlagsEXT.Performance;
#if NET5_0_OR_GREATER
                debugUtilsCreateInfo.pfnUserCallback = &DebugMessengerCallback;
#else
                debugUtilsCreateInfo.pfnUserCallback = Marshal.GetFunctionPointerForDelegate(DebugMessagerCallbackDelegate);
#endif
                createInfo.pNext = &debugUtilsCreateInfo;
            }

            vkCreateInstance(&createInfo, null, out _instance).CheckResult();
            vkLoadInstance(_instance);
        }

        // Find physical device, setup queue's and create device.
        var physicalDevices = vkEnumeratePhysicalDevices(_instance);
        foreach (var physicalDevice in physicalDevices)
        {
            //vkGetPhysicalDeviceProperties(physicalDevice, out var properties);
            //var deviceName = properties.GetDeviceName();
        }

        PhysicalDevice = physicalDevices[0];

        {
            ReadOnlySpan<VkQueueFamilyProperties> queueFamilies = vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice);

            List<string> enabledExtensions = new List<string>
            {
                KHRSwapchainExtensionName
            };

            float priority = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo = new VkDeviceQueueCreateInfo
            {
                sType = VkStructureType.DeviceQueueCreateInfo,
                queueFamilyIndex = 0, // queueFamilies.graphicsFamily,
                queueCount = 1,
                pQueuePriorities = &priority
            };

            using var deviceExtensionNames = new VkStringArray(enabledExtensions);

            VkDeviceCreateInfo createInfo = new VkDeviceCreateInfo
            {
                sType = VkStructureType.DeviceCreateInfo,
                //pNext = &deviceFeatures2,
                queueCreateInfoCount = 1,
                pQueueCreateInfos = &queueCreateInfo,
                enabledExtensionCount = deviceExtensionNames.Length,
                ppEnabledExtensionNames = deviceExtensionNames,
                pEnabledFeatures = null,
            };

            vkCreateDevice(PhysicalDevice, &createInfo, null, out _handle);
        }

        // Init caps
        {
            vkGetPhysicalDeviceProperties(PhysicalDevice, out VkPhysicalDeviceProperties properties);
            vkGetPhysicalDeviceFeatures(PhysicalDevice, out VkPhysicalDeviceFeatures features);

            VendorId = (GpuVendorId)properties.vendorID;
            AdapterId = properties.deviceID;
            AdapterName = properties.GetDeviceName();

            switch (properties.deviceType)
            {
                case VkPhysicalDeviceType.IntegratedGpu:
                    AdapterType = GpuAdapterType.IntegratedGPU;
                    break;

                case VkPhysicalDeviceType.DiscreteGpu:
                    AdapterType = GpuAdapterType.DiscreteGPU;
                    break;

                case VkPhysicalDeviceType.Cpu:
                    AdapterType = GpuAdapterType.CPU;
                    break;

                default:
                    AdapterType = GpuAdapterType.Unknown;
                    break;
            }

            _caps = new GraphicsDeviceCaps()
            {
                Features = new GraphicsDeviceFeatures
                {
                    IndependentBlend = features.independentBlend,
                    ComputeShader = true,
                    TessellationShader = features.tessellationShader,
                    MultiViewport = features.multiViewport,
                    IndexUInt32 = features.fullDrawIndexUint32,
                    MultiDrawIndirect = features.multiDrawIndirect,
                    FillModeNonSolid = features.fillModeNonSolid,
                    SamplerAnisotropy = features.samplerAnisotropy,
                    TextureCompressionETC2 = features.textureCompressionETC2,
                    TextureCompressionASTC_LDR = features.textureCompressionASTC_LDR,
                    TextureCompressionBC = features.textureCompressionBC,
                    TextureCubeArray = features.imageCubeArray,
                    Raytracing = false
                },
                Limits = new GraphicsDeviceLimits
                {
                    MaxVertexAttributes = properties.limits.maxVertexInputAttributes,
                    MaxVertexBindings = properties.limits.maxVertexInputBindings,
                    MaxVertexAttributeOffset = properties.limits.maxVertexInputAttributeOffset,
                    MaxVertexBindingStride = properties.limits.maxVertexInputBindingStride,
                    MaxTextureDimension1D = properties.limits.maxImageDimension1D,
                    MaxTextureDimension2D = properties.limits.maxImageDimension2D,
                    MaxTextureDimension3D = properties.limits.maxImageDimension3D,
                    MaxTextureDimensionCube = properties.limits.maxImageDimensionCube,
                    MaxTextureArrayLayers = properties.limits.maxImageArrayLayers,
                    MaxColorAttachments = properties.limits.maxColorAttachments,
                    MaxUniformBufferRange = properties.limits.maxUniformBufferRange,
                    MaxStorageBufferRange = properties.limits.maxStorageBufferRange,
                    MinUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment,
                    MinStorageBufferOffsetAlignment = properties.limits.minStorageBufferOffsetAlignment,
                    MaxSamplerAnisotropy = (uint)properties.limits.maxSamplerAnisotropy,
                    MaxViewports = properties.limits.maxViewports,
                    MaxViewportWidth = properties.limits.maxViewportDimensions[0],
                    MaxViewportHeight = properties.limits.maxViewportDimensions[1],
                    MaxTessellationPatchSize = properties.limits.maxTessellationPatchSize,
                    MaxComputeSharedMemorySize = properties.limits.maxComputeSharedMemorySize,
                    MaxComputeWorkGroupCountX = properties.limits.maxComputeWorkGroupCount[0],
                    MaxComputeWorkGroupCountY = properties.limits.maxComputeWorkGroupCount[1],
                    MaxComputeWorkGroupCountZ = properties.limits.maxComputeWorkGroupCount[2],
                    MaxComputeWorkGroupInvocations = properties.limits.maxComputeWorkGroupInvocations,
                    MaxComputeWorkGroupSizeX = properties.limits.maxComputeWorkGroupSize[0],
                    MaxComputeWorkGroupSizeY = properties.limits.maxComputeWorkGroupSize[1],
                    MaxComputeWorkGroupSizeZ = properties.limits.maxComputeWorkGroupSize[2],
                }
            };
        }
    }

    public bool DebugUtils { get; }

    public VkInstance Instance => _instance;
    public VkPhysicalDevice PhysicalDevice { get; }
    public VkDevice NativeDevice => _handle;

    // <inheritdoc />
    public override GpuVendorId VendorId { get; }

    /// <inheritdoc />
    public override uint AdapterId { get; }

    /// <inheritdoc />
    public override GpuAdapterType AdapterType { get; }

    /// <inheritdoc />
    public override string AdapterName { get; }

    /// <inheritdoc />
    public override GraphicsDeviceCaps Capabilities => _caps;


    /// <inheritdoc />
    protected override void OnDispose()
    {
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

            // TODO: Enumerate physical devices and try to create instance.

            return true;
        }
        catch
        {
            return false;
        }
    }

#if NET5_0_OR_GREATER
    [UnmanagedCallersOnly]
#endif
    private static uint DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* userData)
    {
        string? message = Interop.GetString(pCallbackData->pMessage);
        if (messageTypes == VkDebugUtilsMessageTypeFlagsEXT.Validation)
        {
            if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Error)
            {
                //Log.Error($"[Vulkan]: Validation: {messageSeverity} - {message}");
            }
            else if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Warning)
            {
                //Log.Warn($"[Vulkan]: Validation: {messageSeverity} - {message}");
            }

            System.Diagnostics.Debug.WriteLine($"[Vulkan]: Validation: {messageSeverity} - {message}");
        }
        else
        {
            if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Error)
            {
                //Log.Error($"[Vulkan]: {messageSeverity} - {message}");
            }
            else if (messageSeverity == VkDebugUtilsMessageSeverityFlagsEXT.Warning)
            {
                //Log.Warn($"[Vulkan]: {messageSeverity} - {message}");
            }

            System.Diagnostics.Debug.WriteLine($"[Vulkan]: {messageSeverity} - {message}");
        }

        return VK_FALSE;
    }


    /// <inheritdoc />
    public override void WaitIdle()
    {
        vkDeviceWaitIdle(_handle).CheckResult();
    }

    /// <inheritdoc />
    protected override SwapChain CreateSwapChainCore(in SwapChainSource source, in SwapChainDescriptor descriptor) => new VulkanSwapChain(this, source, descriptor);

    /// <inheritdoc />
    protected override Texture CreateTextureCore(in TextureDescriptor descriptor) => new VulkanTexture(this, descriptor);
}
