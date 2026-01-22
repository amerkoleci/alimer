// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCopyAllocator : IDisposable
{
    private readonly VulkanGraphicsDevice _device;
    private readonly List<VulkanUploadContext> _freelist = new();

    public VulkanCopyAllocator(VulkanGraphicsDevice device)
    {
        _device = device;
    }

    public void Dispose()
    {
        _device.DeviceApi.vkQueueWaitIdle(_device.VkCopyQueue.Handle);
        foreach (VulkanUploadContext context in _freelist)
        {
            _device.DeviceApi.vkDestroyCommandPool(_device.Handle, context.TransferCommandPool, null);
            _device.DeviceApi.vkDestroyCommandPool(_device.Handle, context.TransitionCommandPool, null);
            for (int i = 0; i < context.Semaphores.Length; i++)
            {
                _device.DeviceApi.vkDestroySemaphore(_device.Handle, context.Semaphores[i], null);
            }
            _device.DeviceApi.vkDestroyFence(_device.Handle, context.Fence, null);

            context.UploadBuffer.Dispose();
        }
    }

    public VulkanUploadContext Allocate(ulong size)
    {
        VulkanUploadContext context = new();

        lock (_freelist)
        {
            // Try to search for a staging buffer that can fit the request:
            for (int i = 0; i < _freelist.Count; ++i)
            {
                if (_freelist[i].UploadBufferSize >= size)
                {
                    if (_device.DeviceApi.vkGetFenceStatus(_device.Handle, _freelist[i].Fence) == VkResult.Success)
                    {
                        context = _freelist[i];
                        VulkanUploadContext temp = _freelist[i];
                        _freelist[i] = _freelist[_freelist.Count - 1];
                        _freelist[_freelist.Count - 1] = temp;
                        _freelist.RemoveAt(_freelist.Count - 1);
                        break;
                    }
                }
            }
        }

        // If no buffer was found that fits the data, create one:
        if (!context.IsValid)
        {
            _device.DeviceApi.vkCreateCommandPool(_device.Handle, VkCommandPoolCreateFlags.Transient, _device.CopyQueueFamily, out context.TransferCommandPool).CheckResult();
            _device.DeviceApi.vkCreateCommandPool(_device.Handle, VkCommandPoolCreateFlags.Transient, _device.GraphicsFamily, out context.TransitionCommandPool).CheckResult();

            _device.DeviceApi.vkAllocateCommandBuffer(_device.Handle, context.TransferCommandPool, out context.TransferCommandBuffer).CheckResult();
            _device.DeviceApi.vkAllocateCommandBuffer(_device.Handle, context.TransitionCommandPool, out context.TransitionCommandBuffer).CheckResult();

            _device.DeviceApi.vkCreateFence(_device.Handle, out context.Fence).CheckResult();

            _device.DeviceApi.vkCreateSemaphore(_device.Handle, out context.Semaphores[0]).CheckResult();
            _device.DeviceApi.vkCreateSemaphore(_device.Handle, out context.Semaphores[1]).CheckResult();
            _device.DeviceApi.vkCreateSemaphore(_device.Handle, out context.Semaphores[2]).CheckResult();

            context.UploadBufferSize = BitOperations.RoundUpToPowerOf2(size);
            context.UploadBufferSize = Math.Max(context.UploadBufferSize, 65536);

            BufferDescriptor uploadBufferDesc = new(context.UploadBufferSize, BufferUsage.None, MemoryType.Upload, "CopyAllocator::UploadBuffer");
            context.UploadBuffer = (VulkanBuffer)_device.CreateBuffer(in uploadBufferDesc);
        }

        // Begin command list in valid state
        _device.DeviceApi.vkResetCommandPool(_device.Handle, context.TransferCommandPool, 0).CheckResult();
        _device.DeviceApi.vkResetCommandPool(_device.Handle, context.TransitionCommandPool, 0).CheckResult();

        VkCommandBufferBeginInfo beginInfo = new()
        {
            flags = VkCommandBufferUsageFlags.OneTimeSubmit,
            pInheritanceInfo = null
        };
        _device.DeviceApi.vkBeginCommandBuffer(context.TransferCommandBuffer, &beginInfo).CheckResult();
        _device.DeviceApi.vkBeginCommandBuffer(context.TransitionCommandBuffer, &beginInfo).CheckResult();

        _device.DeviceApi.vkResetFences(_device.Handle, context.Fence).CheckResult();
        return context;
    }

    public void Submit(in VulkanUploadContext context)
    {
        _device.DeviceApi.vkEndCommandBuffer(context.TransferCommandBuffer).CheckResult();
        _device.DeviceApi.vkEndCommandBuffer(context.TransitionCommandBuffer).CheckResult();


        // Copy queue first
        {
            VkCommandBufferSubmitInfo commandBufferSubmitInfo = new()
            {
                commandBuffer = context.TransferCommandBuffer
            };

            VkSemaphoreSubmitInfo signalSemaphoreInfo = new()
            {
                semaphore = context.Semaphores[0], // Signal for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                commandBufferInfoCount = 1,
                pCommandBufferInfos = &commandBufferSubmitInfo,
                signalSemaphoreInfoCount = 1,
                pSignalSemaphoreInfos = &signalSemaphoreInfo
            };

            lock (_device.VkCopyQueue.LockObject)
            {
                _device.DeviceApi.vkQueueSubmit2(_device.VkCopyQueue.Handle, 1, &submitInfo, VkFence.Null).CheckResult();
            }
        }

        // Graphics 
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                semaphore = context.Semaphores[0], // Wait for copy queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkCommandBufferSubmitInfo commandBufferSubmitInfo = new()
            {
                commandBuffer = context.TransitionCommandBuffer
            };

            VkSemaphoreSubmitInfo* signalSemaphoreInfos = stackalloc VkSemaphoreSubmitInfo[2];
            signalSemaphoreInfos[0] = new();
            signalSemaphoreInfos[1] = new();

            signalSemaphoreInfos[0].semaphore = context.Semaphores[1]; // Signal for compute queue
            signalSemaphoreInfos[0].stageMask = VkPipelineStageFlags2.AllCommands;

            VkSubmitInfo2 submitInfo = new()
            {
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 1,
                pCommandBufferInfos = &commandBufferSubmitInfo,
                signalSemaphoreInfoCount = 1u,
                pSignalSemaphoreInfos = signalSemaphoreInfos
            };

            if (_device.VideoDecodeQueueFamily != VK_QUEUE_FAMILY_IGNORED)
            {
                signalSemaphoreInfos[1].semaphore = context.Semaphores[2]; // Signal for video decode queue
                signalSemaphoreInfos[1].stageMask = VkPipelineStageFlags2.AllCommands;
                submitInfo.signalSemaphoreInfoCount = 2;
            }

            lock (_device.VkGraphicsQueue.LockObject)
            {
                _device.DeviceApi.vkQueueSubmit2(_device.VkGraphicsQueue.Handle, 1, &submitInfo, VkFence.Null).CheckResult();
            }
        }

        // VideoDecode
        if (_device.VideoDecodeQueueFamily != VK_QUEUE_FAMILY_IGNORED)
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                semaphore = context.Semaphores[2], // Wait for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 0,
                pCommandBufferInfos = null,
                signalSemaphoreInfoCount = 0,
                pSignalSemaphoreInfos = null
            };

            lock (_device.VkVideoDecodeQueue!.LockObject)
            {
                _device.DeviceApi.vkQueueSubmit2(_device.VkVideoDecodeQueue!.Handle, 1, &submitInfo, VkFence.Null).CheckResult();
            }
        }

        // This must be final submit in this function because it will also signal a fence for state tracking by CPU!
        // Note: Final submit also signals fence
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                semaphore = context.Semaphores[1], // Wait for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 0,
                pCommandBufferInfos = null,
                signalSemaphoreInfoCount = 0,
                pSignalSemaphoreInfos = null
            };

            lock (_device.VkComputeQueue.LockObject)
            {
                _device.DeviceApi.vkQueueSubmit2(_device.VkComputeQueue.Handle, 1, &submitInfo, context.Fence).CheckResult();
            }
        }

        lock (_freelist)
        {
            _freelist.Add(context);
        }
    }
}

internal struct VulkanUploadContext
{
    public VkCommandPool TransferCommandPool = VkCommandPool.Null;
    public VkCommandBuffer TransferCommandBuffer = VkCommandBuffer.Null;
    public VkCommandPool TransitionCommandPool = VkCommandPool.Null;
    public VkCommandBuffer TransitionCommandBuffer = VkCommandBuffer.Null;
    public VkFence Fence = VkFence.Null;
    public VkSemaphore[] Semaphores = new VkSemaphore[3]; // graphics, compute, video
    public ulong UploadBufferSize;
    public VulkanBuffer UploadBuffer = null!;

    public readonly bool IsValid => TransferCommandBuffer.IsNotNull;

    public VulkanUploadContext()
    {

    }
}
