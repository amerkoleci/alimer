// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanTextureView : TextureView
{
    private VkImageView _handle = VkImageView.Null;
    private readonly int _bindlessReadIndex = InvalidBindlessIndex;
    private readonly int _bindlessReadWriteIndex = InvalidBindlessIndex;

    public VulkanTextureView(VulkanTexture texture, in TextureViewDescriptor descriptor)
        : base(texture, descriptor)
    {
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

        if (texture.Usage.HasFlag(TextureUsage.ShaderRead))
        {
            _bindlessReadIndex = texture.VkDevice.BindlessManager.AllocateBindlessSRV(Handle);
        }

        if (texture.Usage.HasFlag(TextureUsage.ShaderWrite))
        {
            _bindlessReadWriteIndex = texture.VkDevice.BindlessManager.AllocateBindlessUAV(Handle);
        }
    }

    public VulkanTexture VkTexture => (VulkanTexture)Texture;
    public VkImageView Handle => _handle;
    public VkFormat VkFormat { get; }

    /// <inheritdoc />
    public override int BindlessReadIndex => _bindlessReadIndex;

    /// <inheritdoc />
    public override int BindlessReadWriteIndex => _bindlessReadWriteIndex;

    /// <inheitdoc />
    internal override void Destroy()
    {
        VulkanBindlessManager bindlessManager = VkTexture.VkDevice.BindlessManager;

        if (_bindlessReadIndex != InvalidBindlessIndex)
        {
            bindlessManager.SampledImages.Free(_bindlessReadIndex);
        }

        if (_bindlessReadWriteIndex != InvalidBindlessIndex)
        {
            bindlessManager.StorageImages.Free(_bindlessReadWriteIndex);
        }

        VkTexture.VkDevice.DeviceApi.vkDestroyImageView(_handle);
        _handle = VkImageView.Null;
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        VkTexture.VkDevice.SetObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, Handle.Handle, newLabel);
    }
}
