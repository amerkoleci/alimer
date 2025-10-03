// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using Vortice.Vulkan;
using XenoAtom.Collections;
using static Vortice.Vulkan.Vulkan;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanGraphicsManager : GraphicsManager
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly VkInstance _instance;
    private readonly VkInstanceApi _instanceApi;
    private readonly VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;
    private readonly VulkanGraphicsAdapter[] _adapters;

    /// <summary>
    /// Gets value indicating whether Vulkan is supported on this platform.
    /// </summary>
    public static bool IsSupported => s_isSupported.Value;

    /// <inheritdoc/>
    public override GraphicsBackendType BackendType => GraphicsBackendType.Vulkan;

    /// <inheritdoc/>
    public override ReadOnlySpan<GraphicsAdapter> Adapters => _adapters;

    public VulkanGraphicsManager(in GraphicsManagerOptions options)
        : base(in options)
    {
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
            bool hasPortability = false;
            for (int i = 0; i < extensionCount; i++)
            {
                Utf8String extensionName = new(availableInstanceExtensions[i].extensionName);
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
                    hasPortability = true;
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
                    instanceExtensions.Add(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
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
                instanceExtensions.Add(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                instanceExtensions.Add(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            }

            //if (HasHeadlessSurface)
            //{
            //    instanceExtensions.Add(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
            //}

            if (options.ValidationMode != GraphicsValidationMode.Disabled)
            {
                // Determine the optimal validation layers to enable that are necessary for useful debugging
                GetOptimalValidationLayers(ref instanceLayers, availableInstanceLayers);
            }

            if (options.ValidationMode == GraphicsValidationMode.Gpu)
            {
                vkEnumerateInstanceExtensionProperties(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME, out uint propertyCount).CheckResult();
                Span<VkExtensionProperties> availableLayerInstanceExtensions = stackalloc VkExtensionProperties[(int)propertyCount];
                vkEnumerateInstanceExtensionProperties(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME, availableLayerInstanceExtensions).CheckResult();
                for (int i = 0; i < availableLayerInstanceExtensions.Length; i++)
                {
                    fixed (byte* pExtensionName = availableLayerInstanceExtensions[i].extensionName)
                    {
                        Utf8String extensionName = new(pExtensionName);
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
                flags = hasPortability ? VkInstanceCreateFlags.EnumeratePortabilityKHR : VkInstanceCreateFlags.None,
                pApplicationInfo = &appInfo,
                enabledLayerCount = vkLayerNames.Length,
                ppEnabledLayerNames = vkLayerNames,
                enabledExtensionCount = vkInstanceExtensions.Length,
                ppEnabledExtensionNames = vkInstanceExtensions
            };

            if (options.ValidationMode != GraphicsValidationMode.Disabled && DebugUtils)
            {
                debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagsEXT.Error | VkDebugUtilsMessageSeverityFlagsEXT.Warning;
                debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagsEXT.Validation | VkDebugUtilsMessageTypeFlagsEXT.Performance;

                if (options.ValidationMode == GraphicsValidationMode.Verbose)
                {
                    debugUtilsCreateInfo.messageSeverity |= VkDebugUtilsMessageSeverityFlagsEXT.Verbose;
                    debugUtilsCreateInfo.messageSeverity |= VkDebugUtilsMessageSeverityFlagsEXT.Info;
                }

                debugUtilsCreateInfo.pfnUserCallback = &DebugMessengerCallback;
                createInfo.pNext = &debugUtilsCreateInfo;
            }

            VkValidationFeaturesEXT validationFeaturesInfo = new();

            if (options.ValidationMode == GraphicsValidationMode.Gpu && validationFeatures)
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

            result = vkCreateInstance(&createInfo, null, out _instance);
            if (result != VkResult.Success)
            {
                throw new InvalidOperationException($"Failed to create vulkan instance: {result}");
            }
            _instanceApi = GetApi(_instance);

            if (options.ValidationMode != GraphicsValidationMode.Disabled && DebugUtils)
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

        // Enumerate adapters (physical devices)
        _instanceApi.vkEnumeratePhysicalDevices(_instance, out uint adapterCount).CheckResult();
        Span<VkPhysicalDevice> physicalDevices = stackalloc VkPhysicalDevice[(int)adapterCount];
        _instanceApi.vkEnumeratePhysicalDevices(_instance, physicalDevices).CheckResult();

        List<VulkanGraphicsAdapter> adapters = [];
        for (int i = 0; i < adapterCount; i++)
        {
            VkPhysicalDevice physicalDevice = physicalDevices[i];

            // We require minimum 1.2
            VkPhysicalDeviceProperties2 properties = new();
            _instanceApi.vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
            if (properties.properties.apiVersion < VkVersion.Version_1_2)
            {
                continue;
            }

            VulkanPhysicalDeviceExtensions extensions = VulkanPhysicalDeviceExtensions.Query(_instanceApi, physicalDevice);
            if (!extensions.Swapchain)
            {
                continue;
            }

            VulkanGraphicsAdapter adapter = new(this, in physicalDevice, in extensions);
            adapters.Add(adapter);
        }

        _adapters = [.. adapters];
    }

    public VkInstance Instance => _instance;
    public VkInstanceApi InstanceApi => _instanceApi;
    public bool DebugUtils { get; }
    public bool HasXlibSurface { get; }
    public bool HasXcbSurface { get; }
    public bool HasWaylandSurface { get; }
    public bool HasHeadlessSurface { get; }

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            if (_debugMessenger.IsNotNull)
            {
                _instanceApi.vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger);
            }

            _instanceApi.vkDestroyInstance(_instance);
        }
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

    private static void GetOptimalValidationLayers(ref UnsafeList<Utf8String> instanceLayers, Span<VkLayerProperties> availableLayers)
    {
        // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
        UnsafeList<Utf8String> validationLayers =
        [
            VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME
        ];
        if (ValidateLayers(validationLayers, availableLayers))
        {
            instanceLayers.Add(VK_LAYER_KHRONOS_VALIDATION_EXTENSION_NAME);
            return;
        }

        // Otherwise we fallback to using the LunarG meta layer
        validationLayers =
        [
            "VK_LAYER_LUNARG_standard_validation"u8
        ];
        if (ValidateLayers(validationLayers, availableLayers))
        {
            instanceLayers.Add("VK_LAYER_LUNARG_standard_validation"u8);
            return;
        }

        // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
        validationLayers =
        [
            "VK_LAYER_GOOGLE_threading"u8,
            "VK_LAYER_LUNARG_parameter_validation"u8,
            "VK_LAYER_LUNARG_object_tracker"u8,
            "VK_LAYER_LUNARG_core_validation"u8,
            "VK_LAYER_GOOGLE_unique_objects"u8,
        ];

        if (ValidateLayers(validationLayers, availableLayers))
        {
            instanceLayers.Add("VK_LAYER_GOOGLE_threading"u8);
            instanceLayers.Add("VK_LAYER_LUNARG_parameter_validation"u8);
            instanceLayers.Add("VK_LAYER_LUNARG_object_tracker"u8);
            instanceLayers.Add("VK_LAYER_LUNARG_core_validation"u8);
            instanceLayers.Add("VK_LAYER_GOOGLE_unique_objects"u8);
            return;
        }

        // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
        validationLayers =
        [
            "VK_LAYER_LUNARG_core_validation"u8
        ];

        if (ValidateLayers(validationLayers, availableLayers))
        {
            instanceLayers.Add("VK_LAYER_LUNARG_core_validation"u8);
            return;
        }
    }

    private static bool ValidateLayers(UnsafeList<Utf8String> required, Span<VkLayerProperties> availableLayers)
    {
        foreach (Utf8String layer in required)
        {
            bool found = false;
            for (int i = 0; i < availableLayers.Length; i++)
            {
                fixed (byte* pLayerName = availableLayers[i].layerName)
                {
                    Utf8String availableLayer = new(pLayerName);

                    if (availableLayer == layer)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
            {
                Log.Warn($"Validation Layer '{layer}' not found");
                return false;
            }
        }

        return true;
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
}
