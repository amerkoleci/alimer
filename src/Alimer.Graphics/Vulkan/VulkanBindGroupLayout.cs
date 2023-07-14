// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Utilities.MemoryUtilities;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroupLayout : BindGroupLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkDescriptorSetLayout _handle = VkDescriptorSetLayout.Null;
    private readonly VkDescriptorSetLayoutBinding* _layoutBindings;
    private readonly List<VkDescriptorPoolSize> _descriptorPoolSizeInfo = new();

    public VulkanBindGroupLayout(VulkanGraphicsDevice device, in BindGroupLayoutDescription description)
        : base(description)
    {
        _device = device;

        VkDescriptorSetLayoutCreateFlags flags = VkDescriptorSetLayoutCreateFlags.None;
        int bindingCount = description.Entries.Length;
        _layoutBindings = AllocateArray<VkDescriptorSetLayoutBinding>((nuint)bindingCount);

        for (int i = 0; i < bindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];

            _layoutBindings[i] = new VkDescriptorSetLayoutBinding
            {
                binding = entry.Binding,
                descriptorType = entry.Type.ToVk(),
                descriptorCount = 1u,
                stageFlags = entry.Visibility.ToVk()
            };
        }

        VkDescriptorSetLayoutCreateInfo createInfo = new()
        {
            flags = flags,
            bindingCount = (uint)bindingCount,
            pBindings = _layoutBindings
        };

        VkResult result = vkCreateDescriptorSetLayout(device.Handle, &createInfo, null, out _handle);
        if (result != VkResult.Success)
        {
            Log.Error($"Vulkan: Failed to create {nameof(BindGroupLayout)}.");
            return;
        }

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }

        // count the number of descriptors required per type
        Dictionary<VkDescriptorType, uint> poolSizeMap = new();
        for (int i = 0; i < bindingCount; i++)
        {
            if (poolSizeMap.ContainsKey(_layoutBindings[i].descriptorType) == false)
            {
                poolSizeMap[_layoutBindings[i].descriptorType] = 0;
            }

            poolSizeMap[_layoutBindings[i].descriptorType] += _layoutBindings[i].descriptorCount;
        }

        // compute descriptor pool size info
        foreach (var poolSizeIter in poolSizeMap)
        {
            if (poolSizeIter.Value > 0)
            {
                VkDescriptorPoolSize poolSize = new()
                {
                    type = poolSizeIter.Key,
                    descriptorCount = poolSizeIter.Value
                };
                _descriptorPoolSizeInfo.Add(poolSize);
            }
        }
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBindGroupLayout" /> class.
    /// </summary>
    ~VulkanBindGroupLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkDescriptorSetLayout Handle => _handle;

    public ref VkDescriptorSetLayoutBinding GetLayoutBinding(uint index) => ref _layoutBindings[index];

    public ReadOnlySpan<VkDescriptorPoolSize> PoolSizes => CollectionsMarshal.AsSpan(_descriptorPoolSizeInfo);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.DescriptorSetLayout, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_layoutBindings);
        vkDestroyDescriptorSetLayout(_device.Handle, _handle);
    }
}
