// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    internal unsafe class SwapChainVulkan : SwapChain
    {
        public SwapChainVulkan(GraphicsDeviceVulkan device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
            : base(device, descriptor)
        {
            Surface = VulkanDeviceHelper.CreateSurface(surface);

            Resize(descriptor.Size.Width, descriptor.Size.Height);
        }

        public VkSurfaceKHR Surface { get; }

        public VkSwapchainKHR Handle { get; private set; } = VkSwapchainKHR.Null;

        public void Resize(int width, int height)
        {
            VkDevice vkDevice = ((GraphicsDeviceVulkan)Device).NativeDevice;

            var createInfo = new VkSwapchainCreateInfoKHR
            {
                sType = VkStructureType.SwapchainCreateInfoKHR,
                surface = Surface,
                //minImageCount = imageCount,
                //imageFormat = surfaceFormat.format,
                //imageColorSpace = surfaceFormat.colorSpace,
                //imageExtent = Extent,
                //imageArrayLayers = 1,
                //imageUsage = VkImageUsageFlags.ColorAttachment,
                //imageSharingMode = VkSharingMode.Exclusive,
                //preTransform = swapChainSupport.Capabilities.currentTransform,
                //compositeAlpha = VkCompositeAlphaFlagsKHR.Opaque,
                //presentMode = presentMode,
                clipped = true,
                oldSwapchain = Handle
            };

            vkCreateSwapchainKHR(vkDevice, &createInfo, null, out VkSwapchainKHR swapChain).CheckResult();
            Handle = swapChain;
        }

        /// <inheritdoc />
        public override void Present() => throw new System.NotImplementedException();

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
            }
        }
    }
}
