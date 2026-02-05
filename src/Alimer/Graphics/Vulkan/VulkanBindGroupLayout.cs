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
    private readonly VkDescriptorSetLayout _handle = VkDescriptorSetLayout.Null;
    private readonly VkDescriptorSetLayoutBinding* _bindings;

    public VulkanBindGroupLayout(VulkanGraphicsDevice device, in BindGroupLayoutDescriptor description)
        : base(description)
    {
        VkDevice = device;

        VkDescriptorSetLayoutCreateFlags flags = VkDescriptorSetLayoutCreateFlags.None;
        LayoutBindingCount = description.Entries.Length;
        _bindings = AllocateArray<VkDescriptorSetLayoutBinding>((nuint)LayoutBindingCount);

        bool isPush = false;
        if (isPush)
            flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;

        for (int i = 0; i < LayoutBindingCount; i++)
        {
            ref readonly BindGroupLayoutEntry entry = ref description.Entries[i];
            uint registerOffset;

            if (entry.StaticSampler.HasValue)
            {
                registerOffset = device.GetRegisterOffset(VK_DESCRIPTOR_TYPE_SAMPLER, false);
                VkSampler sampler = device.GetOrCreateVulkanSampler(entry.StaticSampler.Value);

                _bindings[i] = new VkDescriptorSetLayoutBinding
                {
                    binding = entry.Binding + registerOffset,
                    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                    descriptorCount = 1u,
                    stageFlags = VK_SHADER_STAGE_ALL,
                    pImmutableSamplers = &sampler,
                };
                continue;
            }

            bool readOnlyStorage = false;
            VkDescriptorType vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            switch (entry.BindingType)
            {
                case BindingInfoType.Buffer:
                    switch (entry.Buffer.Type)
                    {
                        case BufferBindingType.Constant:
                            if (entry.Buffer.HasDynamicOffset)
                            {
                                vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                            }
                            else
                            {
                                vkDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                            }
                            break;

                        case BufferBindingType.Storage:
                        case BufferBindingType.ReadOnlyStorage:
                            // UniformTexelBuffer, StorageTexelBuffer ?
                            readOnlyStorage = (entry.Buffer.Type == BufferBindingType.ReadOnlyStorage);
                            if (entry.Buffer.HasDynamicOffset)
                            {
                                vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                            }
                            else
                            {
                                vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                            }
                            break;
                    }
                    break;

                case BindingInfoType.Sampler:
                    vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                    break;

                case BindingInfoType.Texture:
                    vkDescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    break;

                case BindingInfoType.StorageTexture:
                    vkDescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                    break;

                case BindingInfoType.AccelerationStructure:
                    vkDescriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                    break;

                default:
                    ThrowHelper.ThrowInvalidOperationException();
                    break;
            }

            registerOffset = device.GetRegisterOffset(vkDescriptorType, readOnlyStorage);
            _bindings[i] = new VkDescriptorSetLayoutBinding
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
            pBindings = _bindings
        };

        VkResult result = VkDevice.DeviceApi.vkCreateDescriptorSetLayout(&createInfo, null, out _handle);
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
    public override GraphicsDevice Device => VkDevice;

    public VulkanGraphicsDevice VkDevice { get; }
    public VkDescriptorSetLayout Handle => _handle;

    public int LayoutBindingCount { get; }
    public ref VkDescriptorSetLayoutBinding GetLayoutBinding(uint index) => ref _bindings[index];

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        VkDevice.SetObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        Free(_bindings);
        VkDevice.DeviceApi.vkDestroyDescriptorSetLayout(_handle);
    }

    /// <inheritdoc />
    protected override BindGroup CreateBindGroupCore(in BindGroupDescriptor descriptor)
    {
        return new VulkanBindGroup(this, descriptor);
    }
}
