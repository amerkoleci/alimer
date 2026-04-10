// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Vortice.Vulkan;
using static Alimer.Utilities.MemoryUtilities;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroupLayout : BindGroupLayout
{
    private readonly VkDescriptorSetLayout _handle = VkDescriptorSetLayout.Null;
    private readonly VkDescriptorSetLayoutBinding* _bindings;

    public VulkanBindGroupLayout(VulkanGraphicsDevice device, in BindGroupLayoutDescriptor description)
        : base(description)
    {
        VkDevice = device;

        LayoutBindingCount = description.Entries.Length;
        _bindings = AllocateArray<VkDescriptorSetLayoutBinding>((uint)LayoutBindingCount);

        for (int i = 0; i < LayoutBindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];
            (VkDescriptorType DescriptorType, uint RegisterOffset) = device.GetVkDescriptorType(entry);

            _bindings[i] = new VkDescriptorSetLayoutBinding
            {
                binding = entry.Binding + RegisterOffset,
                descriptorType = DescriptorType,
                descriptorCount = Math.Max(entry.Count, 1u), // VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK ?
                stageFlags = entry.Visibility.ToVk()
            };
        }

        _handle = device.BindlessManager.BindingsSetLayout;

        if (!string.IsNullOrEmpty(description.Label))
        {
            OnLabelChanged(description.Label!);
        }
    }

    /// <inheritdoc />
    public override GPUDevice Device => VkDevice;

    public VulkanGraphicsDevice VkDevice { get; }
    public VkDescriptorSetLayout Handle => _handle;

    public int LayoutBindingCount { get; }
    public ref VkDescriptorSetLayoutBinding GetLayoutBinding(uint index) => ref _bindings[index];

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        //VkDevice.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_bindings);
        //VkDevice.DeviceApi.vkDestroyDescriptorSetLayout(_handle);
    }

    /// <inheritdoc />
    protected override BindGroup CreateBindGroupCore(in BindGroupDescriptor descriptor)
    {
        return new VulkanBindGroup(this, descriptor);
    }
}
