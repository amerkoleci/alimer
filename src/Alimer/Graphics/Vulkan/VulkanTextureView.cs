// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanTextureView : TextureView
{
    private readonly VulkanTexture _texture;
    private VkImageView _handle = VkImageView.Null;

    public VulkanTextureView(VulkanTexture texture, in TextureViewDescriptor descriptor)
        : base(texture, descriptor)
    {
        _texture = texture;
        VkFormat = texture.VkDevice.VkAdapter.ToVkFormat(descriptor.Format);

        VkImageAspectFlags aspectFlags = VkFormat.GetImageAspectFlags(descriptor.Aspect);
        VkImageViewCreateInfo createInfo = new()
        {
            pNext = null,
            flags = 0,
            image = texture.Handle,
            viewType = descriptor.Dimension.ToVk(),
            format = VkFormat,
            components = descriptor.Swizzle.ToVk(),
            subresourceRange = new VkImageSubresourceRange(aspectFlags,
                descriptor.BaseMipLevel,
                descriptor.MipLevelCount,
                descriptor.BaseArrayLayer,
                descriptor.ArrayLayerCount
                )
        };

        VkResult result = texture.VkDevice.DeviceApi.vkCreateImageView(in createInfo, out _handle);
        if (result != VK_SUCCESS)
        {
            Log.Error($"Vulkan: Failed to create ImageView, error: {result}");
        }
    }

    public VkImageView Handle => _handle;
    public VkFormat VkFormat { get; }

    /// <inheitdoc />
    internal override void Destroy()
    {
        _texture.VkDevice.DeviceApi.vkDestroyImageView(_handle);
        _handle = VkImageView.Null;
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _texture.VkDevice.SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, Handle.Handle, newLabel);
    }
}
