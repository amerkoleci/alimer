// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan
{
    /// <summary>
    /// Vulkan graphics device implementation.
    /// </summary>
    public unsafe class VulkanGraphicsDevice : GraphicsDevice
    {
        private static readonly VkString s_EngineName = "Engine";
        private static readonly string[] s_RequestedValidationLayers = new[] { "VK_LAYER_KHRONOS_validation" };
        private static bool? _supportInitializad;

        private VkInstance _instance;
        private vkDebugUtilsMessengerCallbackEXT? _debugMessengerCallbackFunc;
        private readonly VkDebugUtilsMessengerEXT _debugMessenger = VkDebugUtilsMessengerEXT.Null;
        private GraphicsDeviceCaps _capabilities;

        public static bool IsSupported()
        {
            if (_supportInitializad.HasValue)
                return _supportInitializad.Value;

            try
            {
                VkResult result = vkInitialize();
                _supportInitializad = result == VkResult.Success;
                return _supportInitializad.Value;
            }
            catch
            {
                _supportInitializad = false;
                return false;
            }
        }

        public VulkanGraphicsDevice(GraphicsAdapterType adapterPreference)
        {
            if (!IsSupported())
                throw new NotSupportedException("Vulkan is not supported");

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
        }

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
            }
        }

        private static void FindValidationLayers(List<string> appendTo)
        {
            ReadOnlySpan<VkLayerProperties> availableLayers = vkEnumerateInstanceLayerProperties();

            bool hasLayer = false;
            for (int i = 0; i < s_RequestedValidationLayers.Length; i++)
            {
                for (int j = 0; j < availableLayers.Length; j++)
                    if (s_RequestedValidationLayers[i] == availableLayers[j].GetName())
                    {
                        hasLayer = true;
                        break;
                    }

                if (hasLayer)
                {
                    appendTo.Add(s_RequestedValidationLayers[i]);
                }
                else
                {
                    // TODO: Warn
                }
            }
        }

        private static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            IntPtr userData)
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
    }
}
