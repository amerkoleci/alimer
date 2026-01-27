// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using CommunityToolkit.Diagnostics;
using Vortice.Vulkan;
using static Alimer.Utilities.MemoryUtilities;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBindGroupLayout : BindGroupLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkDescriptorSetLayout _handle = VkDescriptorSetLayout.Null;
    private readonly VkDescriptorSetLayoutBinding* _layoutBindings;

    public VulkanBindGroupLayout(VulkanGraphicsDevice device, in BindGroupLayoutDescriptor description)
        : base(description)
    {
        _device = device;

        VkDescriptorSetLayoutCreateFlags flags = VkDescriptorSetLayoutCreateFlags.None;
        LayoutBindingCount = description.Entries.Length;
        _layoutBindings = AllocateArray<VkDescriptorSetLayoutBinding>((nuint)LayoutBindingCount);

        for (int i = 0; i < LayoutBindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];
            uint registerOffset;

            if (entry.StaticSampler.HasValue)
            {
                registerOffset = device.GetRegisterOffset(VkDescriptorType.Sampler);
                VkSampler sampler = device.GetOrCreateVulkanSampler(entry.StaticSampler.Value);

                _layoutBindings[i] = new VkDescriptorSetLayoutBinding
                {
                    binding = entry.Binding + registerOffset,
                    descriptorType = VkDescriptorType.Sampler,
                    descriptorCount = 1u,
                    stageFlags = VK_SHADER_STAGE_ALL,
                    pImmutableSamplers = &sampler,
                };
                continue;
            }

            VkDescriptorType vkDescriptorType = VkDescriptorType.Sampler;

            switch (entry.BindingType)
            {
                case BindingInfoType.Buffer:
                    switch (entry.Buffer.Type)
                    {
                        case BufferBindingType.Constant:
                            if (entry.Buffer.HasDynamicOffset)
                            {
                                vkDescriptorType = VkDescriptorType.UniformBufferDynamic;
                            }
                            else
                            {
                                vkDescriptorType = VkDescriptorType.UniformBuffer;
                            }
                            break;

                        case BufferBindingType.Storage:
                        case BufferBindingType.ReadOnlyStorage:
                            // UniformTexelBuffer, StorageTexelBuffer ?
                            if (entry.Buffer.HasDynamicOffset)
                            {
                                vkDescriptorType = VkDescriptorType.StorageBufferDynamic;
                            }
                            else
                            {
                                vkDescriptorType = VkDescriptorType.StorageBuffer;
                            }
                            break;
                    }
                    break;

                case BindingInfoType.Sampler:
                    vkDescriptorType = VkDescriptorType.Sampler;
                    break;

                case BindingInfoType.Texture:
                    vkDescriptorType = VkDescriptorType.SampledImage;
                    break;

                case BindingInfoType.StorageTexture:
                    vkDescriptorType = VkDescriptorType.StorageImage;
                    break;

                default:
                    ThrowHelper.ThrowInvalidOperationException();
                    break;
            }

            registerOffset = device.GetRegisterOffset(vkDescriptorType);

            _layoutBindings[i] = new VkDescriptorSetLayoutBinding
            {
                binding = entry.Binding + registerOffset,
                descriptorType = vkDescriptorType,
                descriptorCount = 1u,
                stageFlags = entry.Visibility.ToVk()
            };
        }

        VkDescriptorSetLayoutCreateInfo createInfo = new()
        {
            flags = flags,
            bindingCount = (uint)LayoutBindingCount,
            pBindings = _layoutBindings
        };

        VkResult result = _device.DeviceApi.vkCreateDescriptorSetLayout(device.Handle, &createInfo, null, out _handle);
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

    /// <summary>
    /// Finalizes an instance of the <see cref="VulkanBindGroupLayout" /> class.
    /// </summary>
    ~VulkanBindGroupLayout() => Dispose(disposing: false);

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkDescriptorSetLayout Handle => _handle;

    public int LayoutBindingCount { get; }
    public ref VkDescriptorSetLayoutBinding GetLayoutBinding(uint index) => ref _layoutBindings[index];

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_layoutBindings);
        _device.DeviceApi.vkDestroyDescriptorSetLayout(_device.Handle, _handle);
    }
}
