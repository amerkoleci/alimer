// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    public sealed unsafe class VulkanGraphicsProvider : GraphicsProvider
    {
        private static readonly Lazy<bool> s_isSupported = new Lazy<bool>(CheckIsSupported, isThreadSafe: true);
        private static readonly VkString s_appName = new("Vortice");
        private static readonly VkString s_engineName = new("Vortice");
        private readonly VkInstance _instance;

        public static bool IsSupported() => s_isSupported.Value;

        public VulkanGraphicsProvider(bool validation)
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
            vkCreateInstance(&createInfo, null, out _instance);
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="VulkanGraphicsProvider" /> class.
        /// </summary>
        ~VulkanGraphicsProvider() => Dispose(isDisposing: false);


        /// <inheritdoc />
        protected override void Dispose(bool isDisposing)
        {
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
