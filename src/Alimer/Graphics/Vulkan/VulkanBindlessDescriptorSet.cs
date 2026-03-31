// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindlessDescriptorSet : IDisposable
{
    public VulkanBindlessDescriptorSet(VulkanGraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);

        Device = device;

        // Bindless samplers
        VkPhysicalDeviceVulkan12Properties properties12 = device.VkAdapter.Properties12;
        Samplers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_SAMPLER, Math.Min(BindlessSamplerCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages));
        SampledImages = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages / 2));
        StorageImages = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindStorageImages / 2));
        StorageBuffers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindStorageBuffers));

        // Static Samplers
        //new BindGroupLayoutEntry(SamplerDescriptor.PointClamp, 100, ShaderStages.All),          // SamplerPointClamp
        //new BindGroupLayoutEntry(SamplerDescriptor.PointWrap, 101, ShaderStages.All),           // SamplerPointWrap
        //new BindGroupLayoutEntry(SamplerDescriptor.PointMirror, 102, ShaderStages.All),         // SamplerPointMirror
        //new BindGroupLayoutEntry(SamplerDescriptor.LinearClamp, 103, ShaderStages.All),         // SamplerLinearClamp
        //new BindGroupLayoutEntry(SamplerDescriptor.LinearWrap, 104, ShaderStages.All),          // SamplerLinearWrap
        //new BindGroupLayoutEntry(SamplerDescriptor.LinearMirror, 105, ShaderStages.All),        // SamplerLinearMirror
        //new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicClamp, 106, ShaderStages.All),    // SamplerAnisotropicClamp
        //new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicWrap, 107, ShaderStages.All),     // SamplerAnisotropicWrap
        //new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicMirror, 108, ShaderStages.All),   // SamplerAnisotropicMirror
        //new BindGroupLayoutEntry(SamplerDescriptor.ComparisonDepth, 109, ShaderStages.All)      // SamplerAnisotropicMirror

        DescriptorSetCount = 4;
    }


    public VulkanGraphicsDevice Device { get; }

    public VulkanBindlessDescriptorHeap Samplers { get; }
    public VulkanBindlessDescriptorHeap SampledImages { get; }
    public VulkanBindlessDescriptorHeap StorageImages { get; }
    public VulkanBindlessDescriptorHeap StorageBuffers { get; }
    public int DescriptorSetCount { get; }

    public void Dispose()
    {
        Samplers.Dispose();
        SampledImages.Dispose();
        StorageImages.Dispose();
        StorageBuffers.Dispose();
        GC.SuppressFinalize(this);
    }

    public void Bind(VkCommandBuffer commandBuffer, VulkanPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint)
    {
        uint firstSet = (uint)pipelineLayout.BindlessLayoutFirstIndex;
        Span<VkDescriptorSet> descriptorSets = stackalloc VkDescriptorSet[DescriptorSetCount];
        descriptorSets[0] = Samplers.DescriptorSet;
        descriptorSets[1] = SampledImages.DescriptorSet;
        descriptorSets[2] = StorageImages.DescriptorSet;
        descriptorSets[3] = StorageBuffers.DescriptorSet;

        Device.DeviceApi.vkCmdBindDescriptorSets(
            commandBuffer,
            bindPoint,
            pipelineLayout.Handle,
            firstSet,
            descriptorSets
        );
    }

    public int AllocateBindlessSRV(VkBuffer buffer, ulong offset = 0, ulong range = VK_WHOLE_SIZE)
    {
        int bindlessIndex = StorageBuffers.Allocate();
        if (bindlessIndex == InvalidBindlessIndex)
            return InvalidBindlessIndex;

        VkDescriptorBufferInfo bufferInfo = new()
        {
            buffer = buffer,
            offset = offset,
            range = range
        };

        VkWriteDescriptorSet write = new()
        {
            dstSet = StorageBuffers.DescriptorSet,
            dstBinding = 0,
            dstArrayElement = (uint)bindlessIndex,
            descriptorCount = 1,
            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            pBufferInfo = &bufferInfo
        };

        Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);

        return bindlessIndex;
    }

    public int AllocateBindlessSRV(VkImageView view)
    {
        int bindlessIndex = SampledImages.Allocate();
        if (bindlessIndex == InvalidBindlessIndex)
            return InvalidBindlessIndex;

        VkDescriptorImageInfo imageInfo = new()
        {
            imageView = view,
            imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkWriteDescriptorSet write = new()
        {
            dstSet = SampledImages.DescriptorSet,
            dstBinding = 0,
            dstArrayElement = (uint)bindlessIndex,
            descriptorCount = 1,
            descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            pImageInfo = &imageInfo
        };

        Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);

        return bindlessIndex;
    }

    public int AllocateBindlessUAV(VkImageView view)
    {
        int bindlessIndex = StorageImages.Allocate();
        if (bindlessIndex == InvalidBindlessIndex)
            return InvalidBindlessIndex;

        VkDescriptorImageInfo imageInfo = new()
        {
            imageView = view,
            imageLayout = VK_IMAGE_LAYOUT_GENERAL
        };

        VkWriteDescriptorSet write = new()
        {
            dstSet = StorageImages.DescriptorSet,
            dstBinding = 0,
            dstArrayElement = (uint)bindlessIndex,
            descriptorCount = 1,
            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            pImageInfo = &imageInfo
        };

        Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);

        return bindlessIndex;
    }
}

internal unsafe class VulkanBindlessDescriptorHeap : IDisposable
{
    private readonly Lock _lock = new();
    private readonly List<int> _freeList = [];

    public VulkanBindlessDescriptorHeap(VulkanBindlessDescriptorSet parent, VkDescriptorType type, uint descriptorCount)
    {
        ArgumentNullException.ThrowIfNull(parent);
        Parent = parent;

        DescriptorCount = Math.Min(descriptorCount, 500000u);

        VkDescriptorPoolSize poolSize = new()
        {
            type = type,
            descriptorCount = descriptorCount
        };

        VkDescriptorPoolCreateInfo poolInfo = new()
        {
            poolSizeCount = 1,
            pPoolSizes = &poolSize,
            maxSets = 1,
            flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT
        };

        parent.Device.DeviceApi.vkCreateDescriptorPool(in poolInfo, out VkDescriptorPool descriptorPool).CheckResult();
        DescriptorPool = descriptorPool;

        // DescriptorLayout now
        VkDescriptorSetLayoutBinding setLayoutBinding = new()
        {
            descriptorType = type,
            binding = 0,
            descriptorCount = descriptorCount,
            stageFlags = VK_SHADER_STAGE_ALL,
            pImmutableSamplers = null
        };

        VkDescriptorBindingFlags bindingFlags =
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
            | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
            | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT
            | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
            ;
        VkDescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlagsCreateInfo = new()
        {
            bindingCount = 1,
            pBindingFlags = &bindingFlags
        };

        VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = new()
        {
            pNext = &setLayoutBindingFlagsCreateInfo,
            flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            bindingCount = 1,
            pBindings = &setLayoutBinding
        };
        parent.Device.DeviceApi.vkCreateDescriptorSetLayout(in setLayoutCreateInfo, out VkDescriptorSetLayout descriptorSetLayout).CheckResult();
        DescriptorSetLayout = descriptorSetLayout;

        VkDescriptorSetVariableDescriptorCountAllocateInfo setVariableDescriptorCountAllocateInfo = new()
        {
            descriptorSetCount = 1,
            pDescriptorCounts = &descriptorCount
        };

        VkDescriptorSetAllocateInfo allocInfo = new()
        {
            pNext = &setVariableDescriptorCountAllocateInfo,
            descriptorPool = descriptorPool,
            descriptorSetCount = 1,
            pSetLayouts = &descriptorSetLayout,
        };
        VkDescriptorSet descriptorSet = default;
        parent.Device.DeviceApi.vkAllocateDescriptorSets(&allocInfo, &descriptorSet).CheckResult();
        DescriptorSet = descriptorSet;

        for (int i = 0; i < descriptorCount; ++i)
        {
            _freeList.Add((int)descriptorCount - i - 1);
        }

        // Descriptor safety feature:
        //	We init null descriptors for bindless index = 0 for access safety
        //	Because shader compiler sometimes incorrectly loads descriptor outside of safety branch
        //	Note: these are never freed, this is intentional
        if (type != VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
        {
            int index = Allocate();
            Debug.Assert(index == 0, "Descriptor safety feature error: descriptor index must be 0!");

            VkWriteDescriptorSet write = new()
            {
                sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                dstSet = descriptorSet,
                dstBinding = 0,
                dstArrayElement = (uint)index,
                descriptorCount = 1,
                descriptorType = type,
            };

            VkDescriptorImageInfo imageInfo = default;
            VkDescriptorBufferInfo bufferInfo = default;

            switch (type)
            {
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    imageInfo.imageView = parent.Device.NullImage2DView;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    write.pImageInfo = &imageInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    VkBufferView nullBufferiew = parent.Device.NullBufferView;
                    write.pTexelBufferView = &nullBufferiew;
                    break;

                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    bufferInfo.buffer = parent.Device.NullBuffer;
                    bufferInfo.range = VK_WHOLE_SIZE;
                    write.pBufferInfo = &bufferInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    imageInfo.imageView = parent.Device.NullImage2DView;
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                    write.pImageInfo = &imageInfo;
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    imageInfo.sampler = parent.Device.NullSampler;
                    write.pImageInfo = &imageInfo;
                    break;
                default:
                    throw new InvalidOperationException("Descriptor safety feature error: descriptor type not handled!");
            }

            parent.Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);
        } 
    }

    public VulkanBindlessDescriptorSet Parent { get; }
    public uint DescriptorCount { get; }
    public VkDescriptorPool DescriptorPool { get; }
    public VkDescriptorSetLayout DescriptorSetLayout { get; }
    public VkDescriptorSet DescriptorSet { get; }

    public void Dispose()
    {
        Parent.Device.DeviceApi.vkDestroyDescriptorSetLayout(DescriptorSetLayout);
        Parent.Device.DeviceApi.vkDestroyDescriptorPool(DescriptorPool);
        GC.SuppressFinalize(this);
    }

    public int Allocate()
    {
        lock (_lock)
        {
            if (_freeList.Count > 0)
            {
                int index = _freeList[_freeList.Count - 1];
                _freeList.RemoveAt(_freeList.Count - 1);
                return index;
            }
        }

        return InvalidBindlessIndex;
    }

    public void Free(int index)
    {
        if (index == InvalidBindlessIndex)
            return;

        lock (_lock)
        {
            _freeList.Add(index);
        }
    }
}
