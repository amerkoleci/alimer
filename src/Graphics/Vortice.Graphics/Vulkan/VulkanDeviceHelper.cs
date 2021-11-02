// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    internal static unsafe class VulkanDeviceHelper
    {
        private static readonly VkString s_appName = new("Vortice");
        private static readonly VkString s_engineName = new("Vortice");

        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        public static readonly Lazy<VkInstance> Instance = new(CreateInstance);
        public static readonly Lazy<GraphicsDeviceVulkan> DefaultDevice = new(GetDefaultDevice);

        private static VkInstance CreateInstance()
        {
            // Need to initialize 
            vkInitialize().CheckResult();

            VkApplicationInfo appInfo = new()
            {
                sType = VkStructureType.ApplicationInfo,
                pApplicationName = s_appName,
                applicationVersion = new VkVersion(1, 0, 0),
                pEngineName = s_engineName,
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = vkEnumerateInstanceVersion()
            };

            VkInstanceCreateInfo createInfo = new()
            {
                sType = VkStructureType.InstanceCreateInfo,
                pApplicationInfo = &appInfo,
            };

            vkCreateInstance(&createInfo, null, out VkInstance instance).CheckResult();
            return instance;
        }

        private static bool CheckIsSupported()
        {
            VkResult result = vkInitialize();
            if (result != VkResult.Success)
            {
                return false;
            }

            // TODO: Enumerate physical devices and try to create instance.

            return true;
        }

        private static GraphicsDeviceVulkan GetDefaultDevice()
        {
            ReadOnlySpan<VkPhysicalDevice> physicalDevices = vkEnumeratePhysicalDevices(Instance.Value);
            foreach (VkPhysicalDevice physicalDevice in physicalDevices)
            {
                var availableDeviceExtensions = vkEnumerateDeviceExtensionProperties(physicalDevice);

                // TODO: Check if suitable

                return new GraphicsDeviceVulkan(physicalDevice);
            }

            throw new GraphicsException("Vulkan: No suitable GPU found");
        }

        public static VkSurfaceKHR CreateSurface(in SwapChainSurface surface)
        {
            VkSurfaceKHR result = VkSurfaceKHR.Null;
            if (surface is SwapChainSurfaceWin32 surfaceWin32)
            {
                var surfaceCreateInfo = new VkWin32SurfaceCreateInfoKHR
                {
                    sType = VkStructureType.Win32SurfaceCreateInfoKHR,
                    hinstance = surfaceWin32.Hinstance,
                    hwnd = surfaceWin32.Hwnd
                };

                vkCreateWin32SurfaceKHR(Instance.Value, &surfaceCreateInfo, null, out result).CheckResult();
            }

            return result;
        }
    }
}
