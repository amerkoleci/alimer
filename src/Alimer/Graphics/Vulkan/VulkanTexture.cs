// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Vulkan.VmaMemoryUsage;
using static Alimer.Graphics.Vulkan.Vma;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanTexture : Texture
{
    private VkImage _handle = VkImage.Null;
    private VmaAllocation _allocation;

    public VulkanTexture(VulkanGraphicsDevice device, in TextureDescriptor descriptor, TextureData* initialData)
        : base(descriptor)
    {
        VkDevice = device;
        VkFormat = device.VkAdapter.ToVkFormat(descriptor.Format);
        bool isDepthStencil = descriptor.Format.IsDepthStencilFormat();
        VkImageCreateFlags flags = VkImageCreateFlags.None;
        VkImageType imageType = descriptor.Dimension.ToVk();
        VkImageUsageFlags usage = VkImageUsageFlags.None;
        VkExtent3D extent = default;
        uint arrayLayers = 1u;

        switch (descriptor.Dimension)
        {
            case TextureDimension.Texture1D:
                extent = new VkExtent3D(descriptor.Width, 1u, 1u);
                arrayLayers = (uint)ArrayLayers;
                break;

            case TextureDimension.Texture2D:
                extent = new VkExtent3D(descriptor.Width, descriptor.Height, 1u);
                arrayLayers = (uint)ArrayLayers;

                if (descriptor.Width == descriptor.Height &&
                    descriptor.DepthOrArrayLayers >= 6)
                {
                    flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                }
                break;

            case TextureDimension.Texture3D:
                extent = new VkExtent3D(descriptor.Width, descriptor.Height, (uint)Depth);
                arrayLayers = 1u;
                flags |= VkImageCreateFlags.Array2DCompatible;
                break;
        }

        TextureLayout initialLayout = TextureLayout.Undefined;

        if ((descriptor.Usage & TextureUsage.Transient) != 0)
        {
            usage |= VkImageUsageFlags.TransientAttachment;
        }
        else
        {
            usage |= VkImageUsageFlags.TransferSrc;
            usage |= VkImageUsageFlags.TransferDst;
        }

        if ((descriptor.Usage & TextureUsage.ShaderRead) != 0)
        {
            usage |= VkImageUsageFlags.Sampled;
            initialLayout = TextureLayout.ShaderResource;
        }

        if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
        {
            usage |= VkImageUsageFlags.Storage;

            if (descriptor.Format.IsSrgb())
            {
                flags |= VkImageCreateFlags.ExtendedUsage;
            }

            initialLayout = TextureLayout.UnorderedAccess;
        }

        if ((descriptor.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                usage |= VkImageUsageFlags.DepthStencilAttachment;
                initialLayout = TextureLayout.DepthWrite;
            }
            else
            {
                usage |= VkImageUsageFlags.ColorAttachment;
                initialLayout = TextureLayout.RenderTarget;
            }
        }

        if ((descriptor.Usage & TextureUsage.ShadingRate) != 0)
        {
            usage |= VkImageUsageFlags.FragmentShadingRateAttachmentKHR;
        }

        if (!isDepthStencil && (descriptor.Usage & (TextureUsage.ShaderRead | TextureUsage.RenderTarget)) != 0)
        {
            usage |= VkImageUsageFlags.InputAttachment;
        }

#if TODO_SHARED
        bool isShared = false;
        VkExternalMemoryImageCreateInfo externalInfo = new();
        if ((description.Usage & TextureUsage.Shared) != 0)
        {
            isShared = true;

            // Ensure that the handle type is supported.

            VkExternalImageFormatProperties external_props = new();

            VkPhysicalDeviceExternalImageFormatInfo externalFormatInfo = new();
            externalFormatInfo.handleType = VkExternalMemoryHandleTypeFlags.D3D11Texture; // create_info.external.memory_handle_type;

            VkPhysicalDeviceImageFormatInfo2 info = new()
            {
                pNext = &externalFormatInfo,
                format = VkFormat,
                type = imageType,
                tiling = tiling,
                usage = usage,
                flags = flags
            };

            VkImageFormatProperties2 props2 = new()
            {
                pNext = &external_props
            };
            VkResult res = vkGetPhysicalDeviceImageFormatProperties2(device.PhysicalDevice, &info, &props2);

            if (res != VkResult.Success)
            {
                Log.Error($"Vulkan: Image format is not supported for external memory type {externalFormatInfo.handleType}.");
                return;
            }

            bool supportsImport = (external_props.externalMemoryProperties.externalMemoryFeatures & VkExternalMemoryFeatureFlags.Importable) != 0;
            bool supportsExport = (external_props.externalMemoryProperties.externalMemoryFeatures & VkExternalMemoryFeatureFlags.Exportable) != 0;

            //if (!supportsImport && create_info.external)
            //{
            //    LOGE("Attempting to import with handle type #%x, but it is not supported.\n",
            //         create_info.external.memory_handle_type);
            //    return;
            //}
            //else if (!supports_export && !create_info.external)
            //{
            //    LOGE("Attempting to export with handle type #%x, but it is not supported.\n",
            //         create_info.external.memory_handle_type);
            //    return;
            //}

            externalInfo.handleTypes = externalFormatInfo.handleType;
            externalInfo.pNext = info.pNext;
            info.pNext = &externalInfo;
        } 
#endif

        VkImageCreateInfo imageInfo = new()
        {
            flags = flags,
            imageType = imageType,
            format = VkFormat,
            extent = extent,
            mipLevels = (uint)MipLevelCount,
            arrayLayers = arrayLayers,
            samples = SampleCount.ToVk(),
            tiling = VkImageTiling.Optimal,
            usage = usage
        };

        uint* sharingIndices = stackalloc uint[(int)CommandQueueType.Count];
        device.FillImageSharingIndices(ref imageInfo, sharingIndices);

        VmaAllocationCreateInfo memoryInfo = new()
        {
            usage = VMA_MEMORY_USAGE_AUTO,
        };
        VmaAllocationInfo allocationInfo;
        VkResult result = vmaCreateImage(VkDevice.Allocator, &imageInfo, &memoryInfo, out _handle, out _allocation, &allocationInfo);
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create image.");
            return;
        }

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        bool depthOnlyFormat = Format.IsDepthOnlyFormat();

        // Issue data copy on request
        VkImageSubresourceRange subresourceRange = new(
            VkFormat.GetImageAspectFlags(TextureAspect.All),
            0, imageInfo.mipLevels,
            0, imageInfo.arrayLayers
            );

        if (initialData != null)
        {
            if (subresourceRange.aspectMask != VkImageAspectFlags.Color)
            {
                throw new GraphicsException("Cannot initialize non color texture with initial data");
            }

            VulkanUploadContext context = default;
            void* mappedData = null;
            if (descriptor.MemoryType == MemoryType.Upload)
            {
                //mappedData = texture->mapped_data;
            }
            else
            {
                context = VkDevice.Allocate(allocationInfo.size);
                mappedData = context.UploadBuffer.pMappedData;
            }

            PixelFormatInfo formatInfo = descriptor.Format.GetFormatInfo();
            uint blockSize = formatInfo.BlockWidth;

            List<VkBufferImageCopy> copyRegions = [];

            ulong copyOffset = 0;
            uint initDataIndex = 0;
            for (uint layer = 0; layer < imageInfo.arrayLayers; ++layer)
            {
                uint width = imageInfo.extent.width;
                uint height = imageInfo.extent.height;
                uint depth = imageInfo.extent.depth;
                for (uint mip = 0; mip < imageInfo.mipLevels; ++mip)
                {
                    ref TextureData subresourceData = ref initialData[initDataIndex++];
                    uint srcRowPitch = subresourceData.RowPitch;
                    uint srcSlicePitch = subresourceData.SlicePitch;

                    uint numBlocksX = Math.Max(1u, width / blockSize);
                    uint numBlocksY = Math.Max(1u, height / blockSize);
                    uint dstRowPitch = numBlocksX * formatInfo.BytesPerBlock;
                    uint dstSlicePitch = dstRowPitch * numBlocksY;

                    for (uint z = 0; z < depth; ++z)
                    {
                        byte* dstSlice = (byte*)mappedData + copyOffset + dstSlicePitch * z;
                        byte* srcSlice = (byte*)subresourceData.DataPointer + srcSlicePitch * z;
                        for (uint y = 0; y < numBlocksY; ++y)
                        {
                            Unsafe.CopyBlockUnaligned(
                                dstSlice + dstRowPitch * y,
                                srcSlice + srcRowPitch * y,
                                dstRowPitch
                            );
                        }
                    }

                    if (context.IsValid)
                    {
                        VkBufferImageCopy copyRegion = new()
                        {
                            bufferOffset = copyOffset,
                            bufferRowLength = 0,
                            bufferImageHeight = 0
                        };

                        copyRegion.imageSubresource.aspectMask = VkImageAspectFlags.Color;
                        copyRegion.imageSubresource.mipLevel = mip;
                        copyRegion.imageSubresource.baseArrayLayer = layer;
                        copyRegion.imageSubresource.layerCount = 1;

                        copyRegion.imageOffset = VkOffset3D.Zero;
                        copyRegion.imageExtent = new VkExtent3D(width, height, depth);

                        copyRegions.Add(copyRegion);
                    }

                    copyOffset += dstSlicePitch * depth;

                    width = Math.Max(1u, width / 2);
                    height = Math.Max(1u, height / 2);
                    depth = Math.Max(1u, depth / 2);
                }
            }

            if (context.IsValid)
            {
                VkImageLayoutMapping mappingBefore = ConvertImageLayout(TextureLayout.CopyDest, depthOnlyFormat);

                VkImageMemoryBarrier2 barrier = new()
                {
                    srcStageMask = VkPipelineStageFlags2.AllCommands,
                    srcAccessMask = 0,
                    dstStageMask = mappingBefore.StageFlags,
                    dstAccessMask = mappingBefore.AccessMask,
                    oldLayout = imageInfo.initialLayout,
                    newLayout = VkImageLayout.TransferDstOptimal,
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    image = Handle,
                    subresourceRange = subresourceRange,
                };

                VkDependencyInfo dependencyInfo = new()
                {
                    imageMemoryBarrierCount = 1,
                    pImageMemoryBarriers = &barrier
                };

                VkDevice.DeviceApi.vkCmdPipelineBarrier2(context.TransferCommandBuffer, &dependencyInfo);

                VkDevice.DeviceApi.vkCmdCopyBufferToImage(
                    context.TransferCommandBuffer,
                    context.UploadBuffer!.Handle,
                    Handle,
                    VkImageLayout.TransferDstOptimal,
                    copyRegions.ToArray()
                );

                VkImageLayoutMapping mappingAfter = ConvertImageLayout(initialLayout, depthOnlyFormat);
                Debug.Assert(mappingAfter.Layout != VkImageLayout.Undefined);

                GraphicsUtilities.Swap(ref barrier.srcStageMask, ref barrier.dstStageMask);
                barrier.srcAccessMask = VkAccessFlags2.TransferWrite;
                barrier.dstAccessMask = mappingAfter.AccessMask;
                barrier.oldLayout = VkImageLayout.TransferDstOptimal;
                barrier.newLayout = mappingAfter.Layout;
                VkDevice.DeviceApi.vkCmdPipelineBarrier2(
                    context.TransitionCommandBuffer,
                    &dependencyInfo
                    );

                device.Submit(in context);
            }
        }
        else if (initialLayout != TextureLayout.Undefined)
        {
            VulkanUploadContext context = VkDevice.Allocate(0);

            VkImageLayoutMapping mappingAfter = ConvertImageLayout(initialLayout, depthOnlyFormat);
            Debug.Assert(mappingAfter.Layout != VkImageLayout.Undefined);

            VkImageMemoryBarrier2 barrier = new()
            {
                srcStageMask = VkPipelineStageFlags2.Transfer,
                srcAccessMask = 0,
                dstStageMask = VkPipelineStageFlags2.AllCommands,
                dstAccessMask = mappingAfter.AccessMask,
                oldLayout = imageInfo.initialLayout,
                newLayout = mappingAfter.Layout,
                srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                image = Handle,
                subresourceRange = subresourceRange
            };

            VkDependencyInfo dependencyInfo = new()
            {
                imageMemoryBarrierCount = 1,
                pImageMemoryBarriers = &barrier
            };

            VkDevice.DeviceApi.vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);

            VkDevice.Submit(in context);

            SetTextureLayout(initialLayout);
        }
    }

    public VulkanTexture(VulkanGraphicsDevice device, VkImage existingTexture, in TextureDescriptor descriptor, TextureLayout initialLayout)
        : base(descriptor)
    {
        VkDevice = device;
        _handle = existingTexture;
        VkFormat = device.VkAdapter.ToVkFormat(descriptor.Format);

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

        SetTextureLayout(initialLayout);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => VkDevice;

    public VulkanGraphicsDevice VkDevice { get; }
    public VkImage Handle => _handle;
    public VkFormat VkFormat { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanTexture" /> class.
    /// </summary>
    ~VulkanTexture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        DestroyViews();

        if (_allocation.IsNotNull)
        {
            vmaDestroyImage(VkDevice.Allocator, _handle, _allocation);
            _allocation = VmaAllocation.Null;
        }

        _handle = VkImage.Null;
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        VkDevice.SetObjectName(VkObjectType.Image, Handle.Handle, newLabel);
    }

    /// <inheritdoc />
    protected override TextureView CreateView(in TextureViewDescriptor descriptor)
    {
        return new VulkanTextureView(this, in descriptor);
    }
}
