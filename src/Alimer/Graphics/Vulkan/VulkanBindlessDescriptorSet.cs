// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.Vulkan.VulkanUtils;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindlessDescriptorSet : IDisposable
{
    private const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;
    private const uint BINDLESS_RESOURCE_CAPACITY = 500000;
    private const uint BINDLESS_SAMPLER_CAPACITY = 256; // it is chosen to be addressable by 8 bits

    public VulkanBindlessDescriptorSet(VulkanGraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device);

        Device = device;

        // Bindless samplers
        VkPhysicalDeviceVulkan12Properties properties12 = device.VkAdapter.Properties12;
        Samplers = new VulkanBindlessDescriptorHeap(this, VK_DESCRIPTOR_TYPE_SAMPLER, Math.Min(BINDLESS_SAMPLER_CAPACITY, properties12.maxDescriptorSetUpdateAfterBindSampledImages));
        DescriptorSetCount = 1;
    }


    public VulkanGraphicsDevice Device { get; }

    public VulkanBindlessDescriptorHeap Samplers { get; }
    public int DescriptorSetCount { get; }

    public void Dispose()
    {
        Samplers.Dispose();
        GC.SuppressFinalize(this);
    }

    public void Bind(VkCommandBuffer commandBuffer, VulkanPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint)
    {
        uint firstSet = (uint)pipelineLayout.BindlessLayoutFirstIndex;
        Span<VkDescriptorSet> descriptorSets = stackalloc VkDescriptorSet[DescriptorSetCount];
        descriptorSets[0] = Samplers.DescriptorSet;

        Device.DeviceApi.vkCmdBindDescriptorSets(
            commandBuffer,
            bindPoint,
            pipelineLayout.Handle,
            firstSet,
            descriptorSets
        );
    }
}

internal unsafe class VulkanBindlessDescriptorHeap : IDisposable
{
    private readonly Lock _lock = new();
    private readonly List<uint> _freeList = [];

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

        for (uint i = 0; i < descriptorCount; ++i)
        {
            _freeList.Add(descriptorCount - i - 1);
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

    public uint Allocate()
    {
        lock (_lock)
        {
            if (_freeList.Count > 0)
            {
                uint index = _freeList[_freeList.Count - 1];
                _freeList.RemoveAt(_freeList.Count - 1);
                return index;
            }
        }

        return uint.MaxValue;
    }

    public void Free(uint index)
    {
        if (index == uint.MaxValue)
            return;

        lock (_lock)
        {
            _freeList.Add(index);
        }
    }
}
