// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroupLayout : BindGroupLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkDescriptorSetLayout _handle = VkDescriptorSetLayout.Null;

    public VulkanBindGroupLayout(VulkanGraphicsDevice device, in BindGroupLayoutDescription description)
        : base(description)
    {
        _device = device;

        VkDescriptorSetLayoutCreateFlags flags = VkDescriptorSetLayoutCreateFlags.None;
        int bindingCount = description.Entries.Length;
        VkDescriptorSetLayoutBinding* pBindings = stackalloc VkDescriptorSetLayoutBinding[bindingCount];

        for (int i = 0; i < bindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];

            pBindings[i] = new VkDescriptorSetLayoutBinding
            {
                binding = entry.ShaderRegister,
                descriptorType = entry.Type.ToVk(),
                descriptorCount = 1u,
                stageFlags = entry.Visibility.ToVk()
            };
        }

        VkDescriptorSetLayoutCreateInfo createInfo = new()
        {
            flags = flags,
            bindingCount = (uint)bindingCount,
            pBindings = pBindings
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
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkDescriptorSetLayout Handle => _handle;

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBindGroupLayout" /> class.
    /// </summary>
    ~VulkanBindGroupLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        _device.SetObjectName(VkObjectType.DescriptorSetLayout, _handle.Handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        vkDestroyDescriptorSetLayout(_device.Handle, _handle);
    }
}
