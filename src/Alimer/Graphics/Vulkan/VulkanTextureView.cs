// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanTextureView : TextureView
{
    private readonly VulkanTexture _texture;
    private VkImageView _handle = VkImageView.Null;
    private readonly int _bindlessSRVIndex = InvalidBindlessIndex;

    public VulkanTextureView(VulkanTexture texture, in TextureViewDescriptor descriptor)
        : base(texture, descriptor)
    {
        _texture = texture;
        VkFormat = texture.VkDevice.VkAdapter.ToVkFormat(descriptor.Format);

        uint arrayLayerCount = descriptor.ArrayLayerCount;
        if (descriptor.Dimension == TextureViewDimension.ViewCube
            || descriptor.Dimension == TextureViewDimension.ViewCubeArray)
        {
            arrayLayerCount *= 6;
        }

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
                arrayLayerCount
                )
        };

        VkResult result = texture.VkDevice.DeviceApi.vkCreateImageView(in createInfo, out _handle);
        if (result != VK_SUCCESS)
        {
            Log.Error($"Vulkan: Failed to create ImageView, error: {result}");
        }

        if (texture.VkDevice.Bindless)
        {
            if (texture.Usage.HasFlag(TextureUsage.ShaderRead))
            {
                _bindlessSRVIndex = texture.VkDevice.BindlessDescriptorSet!.SampledImages.Allocate();

                if (_bindlessSRVIndex != InvalidBindlessIndex)
                {
                    VkDescriptorImageInfo imageInfo = new()
                    {
                        imageView = _handle,
                        imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };

                    VkWriteDescriptorSet write = new()
                    {
                        dstSet = texture.VkDevice.BindlessDescriptorSet!.SampledImages.DescriptorSet,
                        dstBinding = 0,
                        dstArrayElement = (uint)_bindlessSRVIndex,
                        descriptorCount = 1,
                        descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        pImageInfo = &imageInfo
                    };

                    texture.VkDevice.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);
                }
            }

            if (texture.Usage.HasFlag(TextureUsage.ShaderWrite))
            {
            }
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
