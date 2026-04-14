// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Utilities.MemoryUtilities;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanPipelineLayout : PipelineLayout
{
    private readonly VulkanGraphicsDevice _device;
    private readonly VkPipelineLayout _handle = VkPipelineLayout.Null;

    public VulkanPipelineLayout(VulkanGraphicsDevice device, in PipelineLayoutDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;

        BindlessLayoutFirstIndex = descriptor.BindGroupLayouts.Length;
        _handle = _device.BindlessManager.PipelineLayout;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    public VkPipelineLayout Handle => _handle;

    public int BindlessLayoutFirstIndex { get; }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        //_device.SetObjectName(VkObjectType.PipelineLayout, _handle, newLabel);
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        //_device.DeviceApi.vkDestroyPipelineLayout(_handle);
    }
}
