// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Vulkan;

internal static class Extensions
{
    extension(GraphicsBuffer buffer)
    {
        internal VulkanBuffer ToVk()
        {
            return (VulkanBuffer)buffer;
        }
    }

    extension(Texture texture)
    {
        internal VulkanTexture ToVk()
        {
            return (VulkanTexture)texture;
        }
    }

    extension(TextureView textureView)
    {
        internal VulkanTextureView ToVk()
        {
            return (VulkanTextureView)textureView;
        }
    }

    extension(Sampler sampler)
    {
        internal VulkanSampler ToVk()
        {
            return (VulkanSampler)sampler;
        }
    }

    extension(QueryHeap queryHeap)
    {
        internal VulkanQueryHeap ToVk()
        {
            return (VulkanQueryHeap)queryHeap;
        }
    }
}
