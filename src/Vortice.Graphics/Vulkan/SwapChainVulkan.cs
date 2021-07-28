// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics.Vulkan
{
    internal unsafe class SwapChainVulkan : SwapChain
    {
        private readonly VkSwapchainKHR _handle;

        public SwapChainVulkan(GraphicsDeviceVulkan device, in SwapChainSurface surface, in SwapChainDescriptor descriptor)
            : base(device, surface, descriptor)
        {
        }

        public VkSwapchainKHR Handle => _handle;

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
