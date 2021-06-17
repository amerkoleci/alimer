// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    public static unsafe class VulkanDeviceHelper
    {
        private static readonly VkString s_appName = new("Vortice");
        private static readonly VkString s_engineName = new("Vortice");

        public static readonly Lazy<bool> IsSupported = new(CheckIsSupported);

        public static readonly Lazy<VkInstance> Instance = new(CreateInstance);

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
    }
}
