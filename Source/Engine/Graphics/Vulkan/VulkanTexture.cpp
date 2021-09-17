// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "VulkanTexture.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphics.h"

namespace Alimer
{
    namespace
    {
        [[nodiscard]] constexpr VkImageType ToVulkan(TextureType type)
        {
            switch (type)
            {
            case TextureType::Texture1D:
                return VK_IMAGE_TYPE_1D;

            case TextureType::Texture2D:
            case TextureType::TextureCube:
                return VK_IMAGE_TYPE_2D;

            case TextureType::Texture3D:
                return VK_IMAGE_TYPE_3D;
            default:
                ALIMER_UNREACHABLE();
                return VK_IMAGE_TYPE_1D;
            }
        }

        [[nodiscard]] constexpr VkImageViewType VulkanImageViewType(TextureType type, bool isArray)
        {
            switch (type)
            {
            case TextureType::Texture1D:
                return isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            case TextureType::Texture2D:
                return isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            case TextureType::Texture3D:
                ALIMER_ASSERT(isArray == false);
                return VK_IMAGE_VIEW_TYPE_3D;
            case TextureType::TextureCube:
                return isArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;

            default:
                ALIMER_UNREACHABLE();
                return VK_IMAGE_VIEW_TYPE_2D;
            }
        }

    }

    VulkanTexture::VulkanTexture(VulkanGraphics& device, const TextureCreateInfo& info, VkImage existingImage, const void* initialData)
        : Texture(info)
        , device{ device }
        , handle(existingImage)
        , allocation{ nullptr }
    {
        if (existingImage == VK_NULL_HANDLE)
        {
            VmaAllocationCreateInfo memoryInfo = {};
            memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
            imageInfo.imageType = ToVulkan(info.type);
            imageInfo.format = ToVulkanFormat(info.format);
            imageInfo.extent.width = info.width;
            imageInfo.extent.height = info.height;

            if (info.type == TextureType::Texture3D)
            {
                imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
                imageInfo.extent.depth = info.depthOrArraySize;
                imageInfo.arrayLayers = 1u;
            }
            else if (info.type == TextureType::TextureCube)
            {
                imageInfo.extent.depth = 1;
                imageInfo.arrayLayers = info.depthOrArraySize * 6;
                imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            }
            else
            {
                imageInfo.extent.depth = 1;
                imageInfo.arrayLayers = info.depthOrArraySize;
            }

            imageInfo.mipLevels = info.mipLevels;
            imageInfo.samples = VulkanSampleCount(info.sampleCount);
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            if (CheckBitsAny(info.usage, TextureUsage::Sampled))
            {
                imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            }
            if (CheckBitsAny(info.usage, TextureUsage::Storage))
            {
                imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
            }
            if (CheckBitsAny(info.usage, TextureUsage::RenderTarget))
            {
                if (IsDepthStencilFormat(info.format))
                {
                    imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                }
                else
                {
                    imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                }

                memoryInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            }

            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VmaAllocationInfo allocationInfo{};
            VkResult result = vmaCreateImage(device.GetAllocator(),
                &imageInfo, &memoryInfo,
                &handle, &allocation, &allocationInfo);

            if (result != VK_SUCCESS)
            {
                VK_LOG_ERROR(result, "Failed to create image.");
                return;
            }

            layout = imageInfo.initialLayout;
            allocatedSize = allocationInfo.size;

            // Upload data.
            if (initialData != nullptr)
            {
                auto uploadContext = device.UploadBegin(allocationInfo.size);

                std::vector<VkBufferImageCopy> copyRegions;

                uint32_t mipWidth = imageInfo.extent.width;
                uint32_t mipHeight = imageInfo.extent.height;
                uint32_t depth = imageInfo.extent.depth;

                const uint8_t* srcMemory = reinterpret_cast<const uint8_t*>(initialData);
                const uint64_t srcTexelSize = GetFormatBlockSize(info.format);
                uint64_t copyOffset = 0;

                for (uint32_t arrayIndex = 0; arrayIndex < imageInfo.arrayLayers; ++arrayIndex)
                {
                    for (uint32_t mipLevel = 0; mipLevel < imageInfo.mipLevels; ++mipLevel)
                    {
                        const uint64_t rowPitch = mipWidth * srcTexelSize;
                        const uint64_t slicePitch = rowPitch * mipHeight;

                        uint64_t copySize = slicePitch * depth * imageInfo.arrayLayers;
                        if (IsBlockCompressedFormat(info.format))
                        {
                            copySize /= 4;
                        }

                        uint8_t* cpyAddress = uploadContext.data + copyOffset;
                        memcpy(cpyAddress, initialData, copySize);

                        VkBufferImageCopy copyRegion = {};
                        copyRegion.bufferOffset = copyOffset;
                        copyRegion.bufferRowLength = 0;
                        copyRegion.bufferImageHeight = 0;
                        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                        copyRegion.imageSubresource.mipLevel = mipLevel;
                        copyRegion.imageSubresource.baseArrayLayer = arrayIndex;
                        copyRegion.imageSubresource.layerCount = 1;
                        copyRegion.imageExtent.width = mipWidth;
                        copyRegion.imageExtent.height = mipHeight;
                        copyRegion.imageExtent.depth = depth;
                        copyRegions.push_back(copyRegion);

                        mipWidth = Max(mipWidth / 2, 1u);
                        mipHeight = Max(1u, height / 2);
                        depth = Max(1u, depth / 2);

                        copyOffset += AlignUp(copySize, srcTexelSize);
                        srcMemory += rowPitch;
                    }
                }

                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.image = handle;
                barrier.oldLayout = imageInfo.initialLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = imageInfo.arrayLayers;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = imageInfo.mipLevels;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                vkCmdPipelineBarrier(
                    uploadContext.commandBuffer,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                vkCmdCopyBufferToImage(
                    uploadContext.commandBuffer,
                    ToVulkan(uploadContext.uploadBuffer)->handle,
                    handle,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    (uint32_t)copyRegions.size(), copyRegions.data());

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(
                    uploadContext.commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                device.UploadEnd(uploadContext);

                layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            OnCreated();
        }

        // Create default view.
        defaultView = GetView();

        if (info.label != nullptr)
        {
            SetName(info.label);
        }
    }

    VulkanTexture::~VulkanTexture()
    {
        Destroy();
    }

    void VulkanTexture::Destroy()
    {
        DestroyViews();

        if (handle != VK_NULL_HANDLE
            && allocation != VK_NULL_HANDLE)
        {
            device.DeferDestroy(handle, allocation);
        }

        handle = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
        OnDestroyed();
    }

    //void VulkanTexture::ApiSetName()
    //{
    //    device.SetObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)handle, name);
    //}

    TextureView* VulkanTexture::CreateView(const TextureViewCreateInfo& createInfo)
    {
        auto result = new VulkanTextureView(this, createInfo);

        if (result->GetHandle() != VK_NULL_HANDLE)
        {
            return result;
        }

        delete result;
        return nullptr;
    }

    void VulkanTexture::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout,
        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
    {
        if (layout == newLayout)
            return;

        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.srcAccessMask = 0u;
        barrier.dstAccessMask = 0u;
        barrier.oldLayout = layout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = handle;
        barrier.subresourceRange.aspectMask = GetVkImageAspectFlags(format);
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = depthOrArraySize;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (layout)
        {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            barrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            // Image is preinitialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newLayout)
        {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (barrier.srcAccessMask == 0)
            {
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        vkCmdPipelineBarrier(commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        layout = newLayout;
    }

    /* VulkanTextureView */
    VulkanTextureView::VulkanTextureView(_In_ VulkanTexture* texture, const TextureViewCreateInfo& info)
        : TextureView(texture, info)
        , device(texture->GetDevice())
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.image = texture->GetHandle();
        createInfo.viewType = VulkanImageViewType(texture->GetTextureType(), texture->GetArraySize() > 1);
        createInfo.format = ToVulkanFormat(format);
        createInfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        createInfo.subresourceRange.aspectMask = GetVkImageAspectFlags(format);
        createInfo.subresourceRange.baseMipLevel = baseMipLevel;
        createInfo.subresourceRange.levelCount = mipLevelCount;
        createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
        createInfo.subresourceRange.layerCount = arrayLayerCount;

        VkResult result = vkCreateImageView(device.GetHandle(), &createInfo, nullptr, &handle);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Failed to create ImageView");
        }

        // Allocate bindless SRV
        if (CheckBitsAny(texture->GetUsage(), TextureUsage::Sampled))
        {
            bindless_srv = device.AllocateSRV();
            if (bindless_srv != -1)
            {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageView = handle;
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
                writeDescriptorSet.dstSet = device.GetBindlessSampledImageDescriptorSet();
                writeDescriptorSet.dstBinding = 0;
                writeDescriptorSet.dstArrayElement = bindless_srv;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writeDescriptorSet.pImageInfo = &imageInfo;
                vkUpdateDescriptorSets(device.GetHandle(), 1, &writeDescriptorSet, 0, nullptr);
            }
        }

        if (CheckBitsAny(texture->GetUsage(), TextureUsage::Storage))
        {
            //bindless_uav = device.AllocateUAV();
            //if (bindless_uav != -1)
            //{
            //    VkDescriptorImageInfo imageInfo = {};
            //    imageInfo.imageView = imageView;
            //    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            //
            //    VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            //    writeDescriptorSet.dstSet = device.GetBindlessDescriptorSet();
            //    writeDescriptorSet.dstBinding = kVulkanStorageBufferBinding;
            //    writeDescriptorSet.dstArrayElement = bindless_srv;
            //    writeDescriptorSet.descriptorCount = 1;
            //    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            //    writeDescriptorSet.pImageInfo = &imageInfo;
            //    vkUpdateDescriptorSets(device.GetHandle(), 1, &writeDescriptorSet, 0, nullptr);
            //}
        }
    }

    VulkanTextureView::~VulkanTextureView()
    {
        if (handle != VK_NULL_HANDLE)
        {
            device.DeferDestroy(handle);
        }

        handle = VK_NULL_HANDLE;
    }
}
