// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vortice.Graphics;

internal unsafe class VulkanTexture : Texture
{
    private readonly VkImage _handle;

    public VulkanTexture(VulkanGraphicsDevice device, in TextureDescriptor descriptor)
        : base(device, descriptor)
    {
        VkImageCreateInfo createInfo = new VkImageCreateInfo()
        {
            sType = VkStructureType.ImageCreateInfo
        };
        vkCreateImage(device.NativeDevice, &createInfo, null, out _handle);
    }

    public VkImage Handle => _handle;

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }
    }
}
