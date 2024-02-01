// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;
#if VMA
using static Vortice.Vulkan.Vma;
#endif

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanTexture : Texture
{
    private readonly VulkanGraphicsDevice _device;
    private readonly Dictionary<int, VkImageView> _views = [];
#if VMA
    private readonly VmaAllocation _allocation = VmaAllocation.Null;
#else
    private VkDeviceMemory _memory = VkDeviceMemory.Null;
#endif

    public VulkanTexture(VulkanGraphicsDevice device, in TextureDescriptor descriptor, TextureData* initialData)
        : base(descriptor)
    {
        _device = device;
        VkFormat = device.ToVkFormat(descriptor.Format);
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
                arrayLayers = descriptor.DepthOrArrayLayers;
                break;

            case TextureDimension.Texture2D:
                extent = new VkExtent3D(descriptor.Width, descriptor.Height, 1u);
                arrayLayers = descriptor.DepthOrArrayLayers;

                if (descriptor.Width == descriptor.Height &&
                    descriptor.DepthOrArrayLayers >= 6)
                {
                    flags |= VkImageCreateFlags.CubeCompatible;
                }
                break;
            case TextureDimension.Texture3D:
                extent = new VkExtent3D(descriptor.Width, descriptor.Height, descriptor.DepthOrArrayLayers);
                arrayLayers = 1u;
                flags |= VkImageCreateFlags.Array2DCompatible;
                break;
        }

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
        }

        if ((descriptor.Usage & TextureUsage.ShaderWrite) != 0)
        {
            usage |= VkImageUsageFlags.Storage;

            if (descriptor.Format.IsSrgb())
            {
                flags |= VkImageCreateFlags.ExtendedUsage;
            }
        }

        if ((descriptor.Usage & TextureUsage.RenderTarget) != 0)
        {
            if (isDepthStencil)
            {
                usage |= VkImageUsageFlags.DepthStencilAttachment;
            }
            else
            {
                usage |= VkImageUsageFlags.ColorAttachment;
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

        bool isShared = false;
#if TODO_SHARED
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
            mipLevels = MipLevelCount,
            arrayLayers = arrayLayers,
            samples = SampleCount.ToVk(),
            tiling = VkImageTiling.Optimal,
            usage = usage
        };

        uint* sharingIndices = stackalloc uint[(int)QueueType.Count];
        device.FillImageSharingIndices(ref imageInfo, sharingIndices);

#if VMA
        VmaAllocationInfo allocationInfo = default;
        VmaAllocationCreateInfo memoryInfo = new()
        {
            usage = VmaMemoryUsage.Auto
        };

        VkResult result = vmaCreateImage(device.MemoryAllocator,
            &imageInfo,
            &memoryInfo,
            out VkImage handle,
            out _allocation,
            &allocationInfo);
#else
        VkResult result = vkCreateImage(device.Handle, &imageInfo, null, out VkImage handle);
#endif
        if (result != VkResult.Success)
        {
            Log.Error("Vulkan: Failed to create image.");
            return;
        }

        Handle = handle;

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }

#if !VMA
        _memory = device.Allocator.AllocateTextureMemory(Handle, isShared, out ulong allocatedSize);
#endif

        // Issue data copy on request
        VkImageSubresourceRange subresourceRange = new(VkFormat.GetVkImageAspectFlags(), 0, imageInfo.mipLevels, 0, imageInfo.arrayLayers);

        if (initialData != null)
        {
            if (subresourceRange.aspectMask != VkImageAspectFlags.Color)
            {
                throw new GraphicsException("Cannot initialize non color texture with initial data");
            }

            VulkanUploadContext context = default;
            void* mappedData = null;
            if (descriptor.CpuAccess == CpuAccessMode.Write)
            {
                //mappedData = texture->mapped_data;
            }
            else
            {
                context = _device.Allocate(allocatedSize);
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
                if (_device.Synchronization2)
                {
                    VkImageMemoryBarrier2 barrier = new()
                    {
                        srcStageMask = VkPipelineStageFlags2.AllCommands,
                        srcAccessMask = 0,
                        dstStageMask = VkPipelineStageFlags2.Transfer,
                        dstAccessMask = VkAccessFlags2.TransferWrite,
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

                    vkCmdPipelineBarrier2(context.TransferCommandBuffer, &dependencyInfo);

                    vkCmdCopyBufferToImage(
                        context.TransferCommandBuffer,
                        context.UploadBuffer!.Handle,
                        Handle,
                        VkImageLayout.TransferDstOptimal,
                        copyRegions
                    );

                    ResourceStateMapping mappingAfter = ConvertResourceState(descriptor.InitialLayout);
                    Debug.Assert(mappingAfter.ImageLayout != VkImageLayout.Undefined);

                    GraphicsUtilities.Swap(ref barrier.srcStageMask, ref barrier.dstStageMask);
                    barrier.srcAccessMask = VkAccessFlags2.TransferWrite;
                    barrier.dstAccessMask = mappingAfter.AccessMask;
                    barrier.oldLayout = VkImageLayout.TransferDstOptimal;
                    barrier.newLayout = mappingAfter.ImageLayout;
                    vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);
                }
                else
                {
                    VkImageMemoryBarrier barrier = new()
                    {
                        srcAccessMask = 0,
                        dstAccessMask = VkAccessFlags.TransferWrite,
                        oldLayout = imageInfo.initialLayout,
                        newLayout = VkImageLayout.TransferDstOptimal,
                        srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        image = Handle,
                        subresourceRange = subresourceRange
                    };

                    vkCmdPipelineBarrier(context.TransferCommandBuffer,
                        VkPipelineStageFlags.AllCommands,
                        VkPipelineStageFlags.Transfer,
                        0,
                        0, null,
                        0, null,
                        1, &barrier
                    );

                    vkCmdCopyBufferToImage(
                        context.TransferCommandBuffer,
                        context.UploadBuffer!.Handle,
                        Handle,
                        VkImageLayout.TransferDstOptimal,
                        copyRegions
                    );

                    ResourceStateMappingLegacy mappingAfter = ConvertResourceStateLegacy(descriptor.InitialLayout);
                    Debug.Assert(mappingAfter.ImageLayout != VkImageLayout.Undefined);

                    barrier.srcAccessMask = VkAccessFlags.TransferWrite;
                    barrier.dstAccessMask = mappingAfter.AccessMask;
                    barrier.oldLayout = VkImageLayout.TransferDstOptimal;
                    barrier.newLayout = mappingAfter.ImageLayout;

                    vkCmdPipelineBarrier(context.TransitionCommandBuffer,
                        VkPipelineStageFlags.Transfer,
                        VkPipelineStageFlags.AllCommands,
                        0,
                        0, null,
                        0, null,
                        1, &barrier
                    );
                }

                device.Submit(in context);
            }
        }
        else if (descriptor.InitialLayout != ResourceStates.Unknown && Handle.IsNotNull)
        {
            VulkanUploadContext context = _device.Allocate(0);
            if (_device.Synchronization2)
            {
                ResourceStateMapping mappingAfter = ConvertResourceState(descriptor.InitialLayout);
                Debug.Assert(mappingAfter.ImageLayout != VkImageLayout.Undefined);

                VkImageMemoryBarrier2 barrier = new()
                {
                    srcStageMask = VkPipelineStageFlags2.Transfer,
                    srcAccessMask = 0,
                    dstStageMask = VkPipelineStageFlags2.AllCommands,
                    dstAccessMask = mappingAfter.AccessMask,
                    oldLayout = imageInfo.initialLayout,
                    newLayout = mappingAfter.ImageLayout,
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

                vkCmdPipelineBarrier2(context.TransitionCommandBuffer, &dependencyInfo);
            }
            else
            {
                ResourceStateMappingLegacy mappingAfter = ConvertResourceStateLegacy(descriptor.InitialLayout);
                Debug.Assert(mappingAfter.ImageLayout != VkImageLayout.Undefined);

                VkImageMemoryBarrier barrier = new()
                {
                    srcAccessMask = 0u,
                    dstAccessMask = mappingAfter.AccessMask,
                    oldLayout = imageInfo.initialLayout,
                    newLayout = mappingAfter.ImageLayout,
                    srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    image = Handle,
                    subresourceRange = subresourceRange
                };

                vkCmdPipelineBarrier(context.TransitionCommandBuffer,
                    VkPipelineStageFlags.Transfer,
                    mappingAfter.StageFlags,
                    0,
                    0, null,
                    0, null,
                    1, &barrier);
            }

            _device.Submit(in context);
        }

        CurrentState = descriptor.InitialLayout;
    }

    public VulkanTexture(VulkanGraphicsDevice device, VkImage existingTexture, in TextureDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        Handle = existingTexture;
        VkFormat = device.ToVkFormat(descriptor.Format);

        if (!string.IsNullOrEmpty(descriptor.Label))
        {
            OnLabelChanged(descriptor.Label!);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public VkImage Handle { get; }
    public VkFormat VkFormat { get; }
    public ResourceStates CurrentState { get; set; }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanTexture" /> class.
    /// </summary>
    ~VulkanTexture() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        foreach (var view in _views.Values)
        {
            vkDestroyImageView(_device.Handle, view);
        }
        _views.Clear();

#if VMA
        if (!_allocation.IsNull)
        {
            vmaDestroyImage(_device.MemoryAllocator, Handle, _allocation);
        }
#else
        vkDestroyImage(_device.Handle, Handle, null);
        if (_memory.IsNotNull)
        {
            _device.Allocator.FreeTextureMemory(_memory);
            _memory = VkDeviceMemory.Null;
        }

#endif
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.Image, Handle.Handle, newLabel);
    }

    public VkImageView GetView(int baseMipLevel, int baseArrayLayer = 0, uint mipLevelCount = VK_REMAINING_MIP_LEVELS, uint arrayLayerCount = VK_REMAINING_ARRAY_LAYERS)
    {
        var hash = HashCode.Combine(baseMipLevel, baseArrayLayer, mipLevelCount, arrayLayerCount);

        if (!_views.TryGetValue(hash, out VkImageView view))
        {
            VkImageAspectFlags aspectFlags = VkFormat.GetVkImageAspectFlags();
            VkImageViewCreateInfo createInfo = new()
            {
                pNext = null,
                flags = 0,
                image = Handle,
                viewType = VkImageViewType.Image2D,
                format = VkFormat,
                components = VkComponentMapping.Identity,
                subresourceRange = new VkImageSubresourceRange(aspectFlags, (uint)baseMipLevel, mipLevelCount, (uint)baseArrayLayer, arrayLayerCount)
            };

            VkResult result = vkCreateImageView(_device.Handle, &createInfo, null, &view);
            if (result != VkResult.Success)
            {
                Log.Error($"Vulkan: Failed to create ImageView, error: {result}");
                return VkImageView.Null;
            }

            _views.Add(hash, view);
        }

        return view;
    }
}
