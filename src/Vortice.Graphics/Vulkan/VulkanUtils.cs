// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Text;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    public static unsafe class VulkanUtils
    {
        private static readonly string[] s_RequestedValidationLayers = new[] { "VK_LAYER_KHRONOS_validation" };

        public static string GetString(byte* stringStart)
        {
            int characters = 0;
            while (stringStart[characters] != 0)
            {
                characters++;
            }

            return Encoding.UTF8.GetString(stringStart, characters);
        }

        public static string[] EnumerateInstanceExtensions()
        {
            uint propCount = 0;
            VkResult result = vkEnumerateInstanceExtensionProperties(null, &propCount, null);
            if (result != VkResult.Success)
            {
                return Array.Empty<string>();
            }

            if (propCount == 0)
            {
                return Array.Empty<string>();
            }

            VkExtensionProperties* properties = stackalloc VkExtensionProperties[(int)propCount];
            vkEnumerateInstanceExtensionProperties(null, &propCount, properties);

            string[] extensions = new string[propCount];
            for (int i = 0; i < propCount; i++)
            {
                extensions[i] = properties[i].GetExtensionName();
            }

            return extensions;
        }

        public static void FindValidationLayers(List<string> appendTo)
        {
            uint propertyCount = 0;
            vkEnumerateInstanceLayerProperties(&propertyCount, null).CheckResult();
            VkLayerProperties* availableLayers = stackalloc VkLayerProperties[(int)propertyCount];
            vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers).CheckResult();

            bool hasLayer = false;
            for (int i = 0; i < s_RequestedValidationLayers.Length; i++)
            {
                for (int j = 0; j < propertyCount; j++)
                {
                    if (s_RequestedValidationLayers[i] == availableLayers[j].GetLayerName())
                    {
                        hasLayer = true;
                        break;
                    }
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
    }
}
