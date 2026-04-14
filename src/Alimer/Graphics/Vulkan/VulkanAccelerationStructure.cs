// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanAccelerationStructure : AccelerationStructure
{
    private readonly VulkanGraphicsDevice _device;
    private VkAccelerationStructureKHR _handle = VkAccelerationStructureKHR.Null;

    public VulkanAccelerationStructure(VulkanGraphicsDevice device, in AccelerationStructureDescriptor descriptor)
        : base(in descriptor)
    {
        _device = device;

        VkAccelerationStructureCreateInfoKHR createInfo = new();

        VkAccelerationStructureKHR handle;
        VkResult result = _device.DeviceApi.vkCreateAccelerationStructureKHR(&createInfo, null, &handle);
        if (result != VK_SUCCESS)
        {
            Log.Error($"Vulkan: Failed to create AccelerationStructure - error: {result}");
        }

        _handle = handle;
    }

    public VkAccelerationStructureKHR Handle => _handle;

    public override GraphicsDevice Device => throw new NotImplementedException();

    /// <inheritdoc />
    protected internal override void Destroy()
    {
        _device.DeviceApi.vkDestroyAccelerationStructureKHR(_handle);
        _handle = VkAccelerationStructureKHR.Null;
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        _device.SetObjectName(VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, Handle.Handle, newLabel);
    }

}
