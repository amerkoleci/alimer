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
    private readonly int _bindlessUAVIndex = InvalidBindlessIndex;

    public VulkanTextureView(VulkanTexture texture, in TextureViewDescriptor descriptor)
        : base(texture, descriptor)
    {
        _texture = texture;
        VkFormat = texture.VkDevice.ToVkFormat(descriptor.Format);

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
                _bindlessSRVIndex = texture.VkDevice.BindlessDescriptorSet!.AllocateBindlessSRV(Handle);
            }

            if (texture.Usage.HasFlag(TextureUsage.ShaderWrite))
            {
                _bindlessUAVIndex = texture.VkDevice.BindlessDescriptorSet!.AllocateBindlessUAV(Handle);
            }
        }
    }

    public VkImageView Handle => _handle;
    public VkFormat VkFormat { get; }

    /// <inheritdoc />
    public override int BindlessShaderReadIndex => _bindlessSRVIndex;

    /// <inheritdoc />
    public override int BindlessShaderWriteIndex => _bindlessUAVIndex;

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
