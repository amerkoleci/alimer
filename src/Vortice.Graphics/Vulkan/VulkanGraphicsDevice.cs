// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Vortice.Graphics.Vulkan.VulkanUtils;

namespace Vortice.Graphics.Vulkan
{
    /// <summary>
    /// Vulkan graphics device implementation.
    /// </summary>
    public unsafe class VulkanGraphicsDevice : GraphicsDevice
    {
        private static readonly VkString s_EngineName = "Alimer";
        private static bool? _supportInitialized;

        private VkInstance _instance;
        private readonly vkDebugUtilsMessengerCallbackEXT? _debugMessengerCallbackFunc;
        private VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;
        private GraphicsDeviceCaps _capabilities;

        public static bool IsSupported()
        {
            if (_supportInitialized.HasValue)
                return _supportInitialized.Value;

            try
            {
                VkResult result = vkInitialize();
                _supportInitialized = result == VkResult.Success;
                return _supportInitialized.Value;
            }
            catch
            {
                _supportInitialized = false;
                return false;
            }
        }

        public VulkanGraphicsDevice(GraphicsAdapterType adapterPreference)
        {
            if (!IsSupported())
            {
                throw new NotSupportedException("Vulkan is not supported");
            }

            HashSet<string> availableInstanceExtensions = new HashSet<string>(EnumerateInstanceExtensions());

            //VkString name = applicationName;
            VkApplicationInfo appInfo = new VkApplicationInfo
            {
                sType = VkStructureType.ApplicationInfo,
                //pApplicationName = name,
                //applicationVersion = new VkVersion(1, 0, 0),
                pEngineName = s_EngineName,
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = vkEnumerateInstanceVersion()
            };

            List<string> instanceExtensions = new List<string>
            {
                KHRSurfaceExtensionName
            };

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                instanceExtensions.Add(KHRWin32SurfaceExtensionName);
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                if (availableInstanceExtensions.Contains(KHRAndroidSurfaceExtensionName))
                {
                    instanceExtensions.Add(KHRAndroidSurfaceExtensionName);
                }
                if (availableInstanceExtensions.Contains(KHRXlibSurfaceExtensionName))
                {
                    instanceExtensions.Add(KHRXlibSurfaceExtensionName);
                }
                if (availableInstanceExtensions.Contains(KHRWaylandSurfaceExtensionName))
                {
                    instanceExtensions.Add(KHRWaylandSurfaceExtensionName);
                }
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                if (availableInstanceExtensions.Contains(EXTMetalSurfaceExtensionName))
                {
                    instanceExtensions.Add(EXTMetalSurfaceExtensionName);
                }
                else // Legacy MoltenVK extensions
                {
                    if (availableInstanceExtensions.Contains(MvkMacosSurfaceExtensionName))
                    {
                        instanceExtensions.Add(MvkMacosSurfaceExtensionName);
                    }
                    if (availableInstanceExtensions.Contains(MvkIosSurfaceExtensionName))
                    {
                        instanceExtensions.Add(MvkIosSurfaceExtensionName);
                    }
                }
            }

            List<string> instanceLayers = new List<string>();
            if (EnableValidation || EnableGPUBasedValidation)
            {
                FindValidationLayers(instanceLayers);
            }

            if (instanceLayers.Count > 0)
            {
                instanceExtensions.Add(EXTDebugUtilsExtensionName);
            }

            using var vkInstanceExtensions = new VkStringArray(instanceExtensions);

            var instanceCreateInfo = new VkInstanceCreateInfo
            {
                sType = VkStructureType.InstanceCreateInfo,
                pApplicationInfo = &appInfo,
                enabledExtensionCount = vkInstanceExtensions.Length,
                ppEnabledExtensionNames = vkInstanceExtensions
            };

            using var vkLayerNames = new VkStringArray(instanceLayers);
            if (instanceLayers.Count > 0)
            {
                instanceCreateInfo.enabledLayerCount = vkLayerNames.Length;
                instanceCreateInfo.ppEnabledLayerNames = vkLayerNames;
            }

            var debugUtilsCreateInfo = new VkDebugUtilsMessengerCreateInfoEXT
            {
                sType = VkStructureType.DebugUtilsMessengerCreateInfoEXT
            };

            if (instanceLayers.Count > 0)
            {
                _debugMessengerCallbackFunc = DebugMessengerCallback;
                debugUtilsCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagsEXT.Error | VkDebugUtilsMessageSeverityFlagsEXT.Warning;
                debugUtilsCreateInfo.messageType = VkDebugUtilsMessageTypeFlagsEXT.Validation | VkDebugUtilsMessageTypeFlagsEXT.Performance;
                debugUtilsCreateInfo.pfnUserCallback = Marshal.GetFunctionPointerForDelegate(_debugMessengerCallbackFunc);

                instanceCreateInfo.pNext = &debugUtilsCreateInfo;
            }

            VkResult result = vkCreateInstance(&instanceCreateInfo, null, out _instance);
            if (result != VkResult.Success)
            {
                throw new InvalidOperationException($"Failed to create vulkan instance: {result}");
            }

            vkLoadInstance(_instance);

            if (instanceLayers.Count > 0)
            {
                vkCreateDebugUtilsMessengerEXT(_instance, &debugUtilsCreateInfo, null, out _debugMessenger).CheckResult();
            }

            //Log.Info($"Created VkInstance with version: {appInfo.apiVersion.Major}.{appInfo.apiVersion.Minor}.{appInfo.apiVersion.Patch}");
            //if (instanceLayers.Count > 0)
            //{
            //    foreach (var layer in instanceLayers)
            //    {
            //        Log.Info($"Instance layer '{layer}'");
            //    }
            //}

            // Find physical device, setup queue's and create device.
            ReadOnlySpan<VkPhysicalDevice> physicalDevices = vkEnumeratePhysicalDevices(_instance);
            foreach (VkPhysicalDevice physicalDevice in physicalDevices)
            {
                vkGetPhysicalDeviceProperties(physicalDevice, out VkPhysicalDeviceProperties properties);
            }

            PhysicalDevice = physicalDevices[0];

            InitCapabilites();
        }

        public VkPhysicalDevice PhysicalDevice { get; }

        /// <inheritdoc/>
        public override GraphicsDeviceCaps Capabilities => _capabilities;

        /// <summary>
        /// Finalizes an instance of the <see cref="VulkanGraphicsDevice" /> class.
        /// </summary>
        ~VulkanGraphicsDevice()
        {
            Dispose(disposing: false);
        }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (_debugMessenger != VkDebugUtilsMessengerEXT.Null)
                {
                    vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, null);
                    _debugMessenger = VkDebugUtilsMessengerEXT.Null;
                }

                if (_instance != VkInstance.Null)
                {
                    vkDestroyInstance(_instance, null);
                }

                _instance = VkInstance.Null;
            }
        }

        private void InitCapabilites()
        {
            vkGetPhysicalDeviceProperties(PhysicalDevice, out VkPhysicalDeviceProperties properties);

            _capabilities.BackendType = BackendType.Vulkan;
            _capabilities.VendorId = new GPUVendorId(properties.vendorID);
            _capabilities.AdapterId = properties.deviceID;

            switch (properties.deviceType)
            {
                case VkPhysicalDeviceType.IntegratedGpu:
                    _capabilities.AdapterType = GraphicsAdapterType.IntegratedGPU;
                    break;

                case VkPhysicalDeviceType.DiscreteGpu:
                    _capabilities.AdapterType = GraphicsAdapterType.DiscreteGPU;
                    break;

                case VkPhysicalDeviceType.Cpu:
                    _capabilities.AdapterType = GraphicsAdapterType.CPU;
                    break;

                default:
                    _capabilities.AdapterType = GraphicsAdapterType.Unknown;
                    break;
            }

            _capabilities.AdapterName = properties.GetDeviceName();
        }


        #region Debug Messenger Callback
        private static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            IntPtr userData)
        {
            string? message = VulkanUtils.GetString(pCallbackData->pMessage);
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

                Debug.WriteLine($"[Vulkan]: Validation: {messageSeverity} - {message}");
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

                Debug.WriteLine($"[Vulkan]: {messageSeverity} - {message}");
            }

            return VkBool32.False;
        }
        #endregion
    }
}