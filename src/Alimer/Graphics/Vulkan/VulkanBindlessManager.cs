// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindlessManager : IDisposable
{
    private enum DescriptorSet
    {
        /// <summary>
        /// Default bindings, space0
        /// </summary>
        Bindings,
        BindlessSampler,
        BindlessSampledImage,
        BindlessStorageImage,
        BindlessStorageBuffer,
        //BindlessUniformTexel,
        //BindlessStorageTexelBuffer,
        //BindlessAcceleationStructure,
        Count,
    }
    private readonly VkDescriptorSetLayout _bindingsSetLayout;
    private readonly VkDescriptorSet[] _descriptorSets;

    public VulkanBindlessManager(VulkanGraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);

        Device = device;

        uint setLayoutCount = 0;
        VkDescriptorSetLayout* pSetLayouts = stackalloc VkDescriptorSetLayout[(int)DescriptorSet.Count];
        VkResult result = VK_SUCCESS;
#if VK_MUTABLE_DESCRIPTOR
        if (MutableDescriptorType)
        {
            _descriptorSets = [];
        }
        else 
#endif
        {
            // Bindless samplers
            VkPhysicalDeviceVulkan12Properties properties12 = device.VkAdapter.Properties12;
            Samplers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_SAMPLER, Math.Min(BindlessSamplerCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages));
            SampledImages = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages / 2));
            StorageImages = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindStorageImages / 2));
            StorageBuffers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindStorageBuffers));
            UniformTexelBuffers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindSampledImages / 2));
            StorageTexelBuffers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, Math.Min(BindlessResourceCapacity, properties12.maxDescriptorSetUpdateAfterBindStorageImages / 2));

            // First set (space0)
            int totalLayoutBindingsWithoutStaticSamplers = DynamicContantBufferCount;
            int totalLayoutBindings = totalLayoutBindingsWithoutStaticSamplers + StaticSamplerCount;
            VkDescriptorSetLayoutBinding* layoutBindings = stackalloc VkDescriptorSetLayoutBinding[totalLayoutBindings];
            VkDescriptorBindingFlags* layoutBindingsFlags = stackalloc VkDescriptorBindingFlags[totalLayoutBindings];

            int bindingIndex = 0;
            for (uint i = 0; i < DynamicContantBufferCount; ++i)
            {
                ref VkDescriptorSetLayoutBinding binding = ref layoutBindings[bindingIndex++];
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                binding.binding = VulkanRegisterShift.ContantBuffer + i;
                binding.descriptorCount = 1;
                binding.stageFlags = VK_SHADER_STAGE_ALL;
            }

            // Static Samplers
            Span<SamplerDescriptor> staticSamplers =
            [
                SamplerDescriptor.PointClamp,
                SamplerDescriptor.PointWrap,
                SamplerDescriptor.PointMirror,
                SamplerDescriptor.LinearClamp,
                SamplerDescriptor.LinearWrap,
                SamplerDescriptor.LinearMirror,
                SamplerDescriptor.AnisotropicClamp,
                SamplerDescriptor.AnisotropicWrap,
                SamplerDescriptor.AnisotropicMirror,
                SamplerDescriptor.ComparisonDepth
            ];

            Span<VkSampler> vkStaticSamplers = stackalloc VkSampler[StaticSamplerCount];
            for (int i = 0; i < StaticSamplerCount; ++i)
            {
                vkStaticSamplers[i] = device.GetOrCreateVulkanSampler(staticSamplers[i]);
            }

            for (int i = 0; i < StaticSamplerCount; ++i)
            {
                ref VkDescriptorSetLayoutBinding binding = ref layoutBindings[bindingIndex++];
                binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                binding.binding = VulkanRegisterShift.Sampler + StaticSamplerRegisterSpaceBegin + (uint)i - 1;
                binding.descriptorCount = 1;
                binding.stageFlags = VK_SHADER_STAGE_ALL;
                binding.pImmutableSamplers = (VkSampler*)Unsafe.AsPointer(ref vkStaticSamplers[i]);
            }

            // Flags for bindings, excluded static samplers
            VkDescriptorBindingFlags bindingFlags =
                //VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
               VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
               ;
            for (uint i = 0; i < totalLayoutBindingsWithoutStaticSamplers; ++i)
            {
                layoutBindingsFlags[i] = bindingFlags;
            }

            VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = new()
            {
                bindingCount = (uint)totalLayoutBindings,
                pBindingFlags = layoutBindingsFlags
            };

            VkDescriptorSetLayoutCreateInfo layoutInfo = new()
            {
                pNext = &bindingFlagsInfo,
                bindingCount = (uint)totalLayoutBindings,
                pBindings = layoutBindings
            };

            result = device.DeviceApi.vkCreateDescriptorSetLayout(in layoutInfo, out _bindingsSetLayout);
            if (result != VK_SUCCESS)
            {
                Log.Error($"Vulkan: Failed to create {nameof(PipelineLayout)}.");
                return;
            }
            device.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, _bindingsSetLayout, "Bindings DescriptorSetLayout");

            // Setup descriptor sets (TODO: Reduce descriptor pool only with what we can bind)
            VkDescriptorSet descriptorSet = VkDescriptorSet.Null;
            result = device.AllocateDescriptorSet(_bindingsSetLayout, &descriptorSet);
            // If we have run out of pool memory and rely on internalPools, create a new internal pool and retry
            if (result == VK_ERROR_OUT_OF_POOL_MEMORY
                || result == VK_ERROR_FRAGMENTED_POOL)
            {
                device.AllocateDescriptorPool();
                result = device.AllocateDescriptorSet(_bindingsSetLayout, &descriptorSet);
                result.CheckResult();
            }
            device.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSet, "Bindings DescriptorSet");

            pSetLayouts[(int)DescriptorSet.Bindings] = _bindingsSetLayout;
            pSetLayouts[(int)DescriptorSet.BindlessSampler] = Samplers.DescriptorSetLayout;
            pSetLayouts[(int)DescriptorSet.BindlessSampledImage] = SampledImages.DescriptorSetLayout;
            pSetLayouts[(int)DescriptorSet.BindlessStorageImage] = StorageImages.DescriptorSetLayout;
            pSetLayouts[(int)DescriptorSet.BindlessStorageBuffer] = StorageBuffers.DescriptorSetLayout;

            setLayoutCount = (uint)DescriptorSet.Count;

            _descriptorSets = new VkDescriptorSet[(int)DescriptorSet.Count];
            _descriptorSets[(int)DescriptorSet.Bindings] = descriptorSet;
            _descriptorSets[(int)DescriptorSet.BindlessSampler] = Samplers.DescriptorSet;
            _descriptorSets[(int)DescriptorSet.BindlessSampledImage] = SampledImages.DescriptorSet;
            _descriptorSets[(int)DescriptorSet.BindlessStorageImage] = StorageImages.DescriptorSet;
            _descriptorSets[(int)DescriptorSet.BindlessStorageBuffer] = StorageBuffers.DescriptorSet;
        }

        VkPushConstantRange pushConstantRange = new()
        {
            stageFlags = VK_SHADER_STAGE_ALL,
            offset = 0u,
            size = PushConstantsSize,
        };

        VkPipelineLayoutCreateInfo createInfo = new()
        {
            setLayoutCount = setLayoutCount,
            pSetLayouts = pSetLayouts,
            pushConstantRangeCount = 1u,
            pPushConstantRanges = &pushConstantRange
        };

        result = device.DeviceApi.vkCreatePipelineLayout(in createInfo, out VkPipelineLayout pipelineLayout);
        if (result != VK_SUCCESS)
        {
            Log.Error($"Vulkan: Failed to create {nameof(PipelineLayout)}.");
            return;
        }
        PipelineLayout = pipelineLayout;
        device.SetObjectName(VK_OBJECT_TYPE_PIPELINE_LAYOUT, pipelineLayout, "Bindless Pipeline Layout");

        DescriptorSetCount = 4;
    }


    public VulkanGraphicsDevice Device { get; }
    //public bool MutableDescriptorType { get; }
    public VkDescriptorSetLayout BindingsSetLayout => _bindingsSetLayout;
    public VkDescriptorSet BindingsDescriptorSet => _descriptorSets[(int)DescriptorSet.Bindings];
    public VkPipelineLayout PipelineLayout { get; }
    public VulkanBindlessDescriptorHeap Samplers { get; }
    public VulkanBindlessDescriptorHeap SampledImages { get; }
    public VulkanBindlessDescriptorHeap StorageImages { get; }
    public VulkanBindlessDescriptorHeap StorageBuffers { get; }
    public VulkanBindlessDescriptorHeap UniformTexelBuffers { get; }
    public VulkanBindlessDescriptorHeap StorageTexelBuffers { get; }

    public int DescriptorSetCount { get; }

    public void Dispose()
    {
        Device.DeviceApi.vkDestroyDescriptorSetLayout(_bindingsSetLayout);
        Device.DeviceApi.vkDestroyPipelineLayout(PipelineLayout);

        Samplers.Dispose();
        SampledImages.Dispose();
        StorageImages.Dispose();
        StorageBuffers.Dispose();
        UniformTexelBuffers.Dispose();
        StorageTexelBuffers.Dispose();
        GC.SuppressFinalize(this);
    }

    public void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, Span<uint> dynamicOffsets)
    {
        Device.DeviceApi.vkCmdBindDescriptorSets(
            commandBuffer,
            bindPoint,
            PipelineLayout.Handle,
            0,
            _descriptorSets,
            dynamicOffsets
        );
    }

    #region BufferView
    public int AllocateStorageBufferView(VkBuffer buffer, ulong offset, ulong range)
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

    public int AllocateUniformTexelBuffer(VkBufferView view)
    {
        int bindlessIndex = UniformTexelBuffers.Allocate();
        if (bindlessIndex == InvalidBindlessIndex)
            return InvalidBindlessIndex;

        VkWriteDescriptorSet write = new()
        {
            dstSet = UniformTexelBuffers.DescriptorSet,
            dstBinding = 0,
            dstArrayElement = (uint)bindlessIndex,
            descriptorCount = 1,
            descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
            pTexelBufferView = &view
        };

        Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);

        return bindlessIndex;
    }

    public int AllocateStorageTexelBuffer(VkBufferView view)
    {
        int bindlessIndex = StorageTexelBuffers.Allocate();
        if (bindlessIndex == InvalidBindlessIndex)
            return InvalidBindlessIndex;

        VkWriteDescriptorSet write = new()
        {
            dstSet = StorageTexelBuffers.DescriptorSet,
            dstBinding = 0,
            dstArrayElement = (uint)bindlessIndex,
            descriptorCount = 1,
            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
            pTexelBufferView = &view
        };

        Device.DeviceApi.vkUpdateDescriptorSets(1, &write, 0, null);

        return bindlessIndex;
    }
    #endregion

    #region Texture
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
    #endregion
}

internal unsafe class VulkanBindlessDescriptorHeap : IDisposable
{
    private readonly Lock _lock = new();
    private readonly List<int> _freeList = [];

    public VulkanBindlessDescriptorHeap(VulkanBindlessManager parent, VkDescriptorType type, uint descriptorCount)
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
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
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
        // We init null descriptors for bindless index = 0 for access safety
        // Because shader compiler sometimes incorrectly loads descriptor outside of safety branch
        // Note: these are never freed, this is intentional
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

    public VulkanBindlessManager Parent { get; }
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
