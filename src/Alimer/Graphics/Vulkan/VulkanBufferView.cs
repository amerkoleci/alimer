// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Alimer.Graphics.Constants;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanBufferView : GPUBufferView
{
    private int _bindlessReadIndex = InvalidBindlessIndex;
    private int _bindlessReadWriteIndex = InvalidBindlessIndex;
    private VkBufferView _handle = VkBufferView.Null;

    public VulkanBufferView(VulkanBuffer buffer, in GPUBufferViewDescriptor descriptor)
        : base(buffer, descriptor)
    {
        ulong offsetInBytes = descriptor.ElementOffset * descriptor.ElementSize;
        ulong sizeInBytes = descriptor.ElementCount * descriptor.ElementSize;

        if (descriptor.ElementFormat == PixelFormat.Undefined)
        {
            _bindlessReadIndex = buffer.VkDevice.BindlessManager.AllocateStorageBufferView(buffer.Handle, offsetInBytes, sizeInBytes);
            _bindlessReadWriteIndex = _bindlessReadIndex;
        }
        else
        {
            // Vulkan BufferView format can't be SRGB
            PixelFormat viewFormat = descriptor.ElementFormat.SrgbToLinearFormat();
            VkBufferViewCreateInfo viewCreateInfo = new()
            {
                sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                buffer = buffer.Handle,
                flags = 0,
                format = buffer.VkDevice.ToVkFormat(viewFormat),
                offset = offsetInBytes,
                range = Math.Min(sizeInBytes, buffer.Size - offsetInBytes)
            };

            VkResult result = buffer.VkDevice.DeviceApi.vkCreateBufferView(in viewCreateInfo, out _handle);
            if (result != VK_SUCCESS)
            {
                Log.Error("Vulkan: Failed to create buffer view.");
                return;
            }

            bool shaderRead = (buffer.Usage & GPUBufferUsage.ShaderRead) != 0;
            bool shaderReadWrite = (buffer.Usage & GPUBufferUsage.ShaderWrite) != 0;

            if (shaderRead)
            {
                _bindlessReadIndex = buffer.VkDevice.BindlessManager.AllocateUniformTexelBuffer(_handle);
            }

            if (shaderReadWrite)
            {
                _bindlessReadWriteIndex = buffer.VkDevice.BindlessManager.AllocateStorageTexelBuffer(_handle);
            }
        }
    }

    public VulkanBuffer VkBuffer => (VulkanBuffer)Buffer;

    /// <inheritdoc />
    public override int BindlessReadIndex => _bindlessReadIndex;

    /// <inheritdoc />
    public override int BindlessReadWriteIndex => _bindlessReadWriteIndex;

    /// <inheritdoc />
    protected override void OnLabelChanged(string? newLabel)
    {
        if (_handle.IsNotNull)
        {
            VkBuffer.VkDevice.SetObjectName(VK_OBJECT_TYPE_BUFFER_VIEW, _handle, newLabel);
        }
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        VulkanBindlessManager bindlessManager = VkBuffer.VkDevice.BindlessManager;

        if (ElementFormat == PixelFormat.Undefined)
        {
            bindlessManager.StorageBuffers.Free(_bindlessReadIndex);
        }
        else
        {
            VkBuffer.VkDevice.DeviceApi.vkDestroyBufferView(_handle);

            if (_bindlessReadIndex != InvalidBindlessIndex)
            {
                bindlessManager.UniformTexelBuffers.Free(_bindlessReadIndex);
            }

            if (_bindlessReadWriteIndex != InvalidBindlessIndex)
            {
                bindlessManager.StorageTexelBuffers.Free(_bindlessReadWriteIndex);
            }
        }

        _bindlessReadIndex = InvalidBindlessIndex;
        _bindlessReadWriteIndex = InvalidBindlessIndex;
        _handle = VkBufferView.Null;
    }
}
