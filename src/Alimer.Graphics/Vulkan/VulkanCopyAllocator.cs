// Copyright © Amer Koleci and Contributors.
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
        vkQueueWaitIdle(_device.CopyQueue.Handle);
        foreach (VulkanUploadContext context in _freelist)
        {
            vkDestroyCommandPool(_device.Handle, context.TransferCommandPool, null);
            vkDestroyCommandPool(_device.Handle, context.TransitionCommandPool, null);
            for (int i = 0; i < context.Semaphores.Length; i++)
            {
                vkDestroySemaphore(_device.Handle, context.Semaphores[i], null);
            }
            vkDestroyFence(_device.Handle, context.Fence, null);

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
                    if (vkGetFenceStatus(_device.Handle, _freelist[i].Fence) == VkResult.Success)
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
            vkCreateCommandPool(_device.Handle, VkCommandPoolCreateFlags.Transient, _device.CopyQueueFamily, out context.TransferCommandPool).DebugCheckResult();
            vkCreateCommandPool(_device.Handle, VkCommandPoolCreateFlags.Transient, _device.GraphicsFamily, out context.TransitionCommandPool).DebugCheckResult();

            vkAllocateCommandBuffer(_device.Handle, context.TransferCommandPool, out context.TransferCommandBuffer).DebugCheckResult();
            vkAllocateCommandBuffer(_device.Handle, context.TransitionCommandPool, out context.TransitionCommandBuffer).DebugCheckResult();

            vkCreateFence(_device.Handle, out context.Fence).DebugCheckResult();

            vkCreateSemaphore(_device.Handle, out context.Semaphores[0]).DebugCheckResult();
            vkCreateSemaphore(_device.Handle, out context.Semaphores[1]).DebugCheckResult();
            vkCreateSemaphore(_device.Handle, out context.Semaphores[2]).DebugCheckResult();

            context.UploadBufferSize = BitOperations.RoundUpToPowerOf2(size);
            context.UploadBufferSize = Math.Max(context.UploadBufferSize, 65536);

            BufferDescription uploadBufferDesc = new(context.UploadBufferSize, BufferUsage.None, CpuAccessMode.Write, "CopyAllocator::UploadBuffer");
            context.UploadBuffer = (VulkanBuffer)_device.CreateBuffer(in uploadBufferDesc);
        }

        // begin command list in valid state:
        vkResetCommandPool(_device.Handle, context.TransferCommandPool, 0).CheckResult();
        vkResetCommandPool(_device.Handle, context.TransitionCommandPool, 0).CheckResult();

        VkCommandBufferBeginInfo beginInfo = new()
        {
            sType = VkStructureType.CommandBufferBeginInfo,
            flags = VkCommandBufferUsageFlags.OneTimeSubmit,
            pInheritanceInfo = null
        };
        vkBeginCommandBuffer(context.TransferCommandBuffer, &beginInfo).DebugCheckResult();
        vkBeginCommandBuffer(context.TransitionCommandBuffer, &beginInfo).DebugCheckResult();

        vkResetFences(_device.Handle, context.Fence).CheckResult();

        return context;
    }

    public void Submit(in VulkanUploadContext context)
    {
        vkEndCommandBuffer(context.TransferCommandBuffer).DebugCheckResult();
        vkEndCommandBuffer(context.TransitionCommandBuffer).DebugCheckResult();

        if (_device.PhysicalDeviceFeatures1_3.synchronization2 == true)
        {
            SubmitSynchronization2(in context);
        }
        else
        {
            SubmitLegacy(in context);
        }

        lock (_freelist)
        {
            _freelist.Add(context);
        }
    }

    private void SubmitSynchronization2(in VulkanUploadContext context)
    {
        // Copy queue first
        {
            VkCommandBufferSubmitInfo commandBufferSubmitInfo = new()
            {
                sType = VkStructureType.CommandBufferSubmitInfo,
                commandBuffer = context.TransferCommandBuffer
            };

            VkSemaphoreSubmitInfo signalSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
                semaphore = context.Semaphores[0], // Signal for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                sType = VkStructureType.SubmitInfo2,
                commandBufferInfoCount = 1,
                pCommandBufferInfos = &commandBufferSubmitInfo,
                signalSemaphoreInfoCount = 1,
                pSignalSemaphoreInfos = &signalSemaphoreInfo
            };

            lock (_device.CopyQueue.LockObject)
            {
                vkQueueSubmit2(_device.CopyQueue.Handle, 1, &submitInfo, VkFence.Null).DebugCheckResult();
            }
        }

        // Graphics 
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
                semaphore = context.Semaphores[0], // Wait for copy queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkCommandBufferSubmitInfo commandBufferSubmitInfo = new()
            {
                sType = VkStructureType.CommandBufferSubmitInfo,
                commandBuffer = context.TransitionCommandBuffer
            };

            VkSemaphoreSubmitInfo* signalSemaphoreInfos = stackalloc VkSemaphoreSubmitInfo[2];
            signalSemaphoreInfos[0].sType = VkStructureType.SemaphoreSubmitInfo;
            signalSemaphoreInfos[1].sType = VkStructureType.SemaphoreSubmitInfo;

            signalSemaphoreInfos[0].semaphore = context.Semaphores[1]; // Signal for compute queue
            signalSemaphoreInfos[0].stageMask = VkPipelineStageFlags2.AllCommands; 

            VkSubmitInfo2 submitInfo = new()
            {
                sType = VkStructureType.SubmitInfo2,
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

            lock (_device.GraphicsQueue.LockObject)
            {
                vkQueueSubmit2(_device.GraphicsQueue.Handle, 1, &submitInfo, VkFence.Null).DebugCheckResult();
            }
        }

        // VideoDecode
        if (_device.VideoDecodeQueueFamily != VK_QUEUE_FAMILY_IGNORED)
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
                semaphore = context.Semaphores[2], // Wait for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                sType = VkStructureType.SubmitInfo2,
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 0,
                pCommandBufferInfos = null,
                signalSemaphoreInfoCount = 0,
                pSignalSemaphoreInfos = null
            };

            lock (_device.VideoDecodeQueue!.LockObject)
            {
                vkQueueSubmit2(_device.VideoDecodeQueue!.Handle, 1, &submitInfo, VkFence.Null).DebugCheckResult();
            }
        }

        // This must be final submit in this function because it will also signal a fence for state tracking by CPU!
        // Note: Final submit also signals fence
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
                semaphore = context.Semaphores[1], // Wait for graphics queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            VkSubmitInfo2 submitInfo = new()
            {
                sType = VkStructureType.SubmitInfo2,
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 0,
                pCommandBufferInfos = null,
                signalSemaphoreInfoCount = 0,
                pSignalSemaphoreInfos = null
            };

            lock (_device.ComputeQueue.LockObject)
            {
                vkQueueSubmit2(_device.ComputeQueue.Handle, 1, &submitInfo, context.Fence).DebugCheckResult();
            }
        }
    }

    private void SubmitLegacy(in VulkanUploadContext context)
    {
        VkCommandBufferSubmitInfo cbSubmitInfo = new();

        VkSemaphoreSubmitInfo* signalSemaphoreInfos = stackalloc VkSemaphoreSubmitInfo[2];
        signalSemaphoreInfos[0].sType = VkStructureType.SemaphoreSubmitInfo;
        signalSemaphoreInfos[1].sType = VkStructureType.SemaphoreSubmitInfo;

        // Copy queue first
        {
            VkCommandBuffer commandBuffer = context.TransferCommandBuffer;
            VkSemaphore semaphore = context.Semaphores[0]; // signal for graphics queue

            VkSubmitInfo submitInfo = new()
            {
                commandBufferCount = 1,
                pCommandBuffers = &commandBuffer,
                signalSemaphoreCount = 1,
                pSignalSemaphores = &semaphore
            };

            lock (_device.CopyQueue.LockObject)
            {
                vkQueueSubmit(_device.CopyQueue.Handle, 1, &submitInfo, VkFence.Null).DebugCheckResult();
            }
        }

        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
                semaphore = context.Semaphores[0], // Wait for copy queue
                stageMask = VkPipelineStageFlags2.AllCommands
            };

            cbSubmitInfo.commandBuffer = context.TransitionCommandBuffer;
            signalSemaphoreInfos[0].semaphore = context.Semaphores[1]; // signal for compute queue
            signalSemaphoreInfos[0].stageMask = VkPipelineStageFlags2.AllCommands; // signal for compute queue

            VkSubmitInfo2 submitInfo = new()
            {
                waitSemaphoreInfoCount = 1,
                pWaitSemaphoreInfos = &waitSemaphoreInfo,
                commandBufferInfoCount = 1,
                pCommandBufferInfos = &cbSubmitInfo,
                pSignalSemaphoreInfos = signalSemaphoreInfos
            };

            //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
            //{
            //    signalSemaphoreInfos[1].semaphore = cmd.semaphores[2]; // signal for video decode queue
            //    signalSemaphoreInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; // signal for video decode queue
            //    submitInfo.signalSemaphoreInfoCount = 2;
            //}
            //else
            {
                submitInfo.signalSemaphoreInfoCount = 1;
            }

            lock (_device.GraphicsQueue.LockObject)
            {
                vkQueueSubmit2(_device.GraphicsQueue.Handle, 1, &submitInfo, VkFence.Null).DebugCheckResult();
            }
        }

        // VideoDecode
        //if (device->queues[QUEUE_VIDEO_DECODE].queue != VK_NULL_HANDLE)
        //{
        //    waitSemaphoreInfo.semaphore = cmd.semaphores[2]; // wait for graphics queue
        //    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        //
        //    submitInfo.waitSemaphoreInfoCount = 1;
        //    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
        //    submitInfo.commandBufferInfoCount = 0;
        //    submitInfo.pCommandBufferInfos = nullptr;
        //    submitInfo.signalSemaphoreInfoCount = 0;
        //    submitInfo.pSignalSemaphoreInfos = nullptr;
        //
        //    std::scoped_lock lock (device->queues[QUEUE_VIDEO_DECODE].locker) ;
        //    res = vkQueueSubmit2(device->queues[QUEUE_VIDEO_DECODE].queue, 1, &submitInfo, VK_NULL_HANDLE);
        //    assert(res == VK_SUCCESS);
        //}

        // This must be final submit in this function because it will also signal a fence for state tracking by CPU!
        // Note: Final submit also signals fence
        {
            VkSemaphoreSubmitInfo waitSemaphoreInfo = new()
            {
                sType = VkStructureType.SemaphoreSubmitInfo,
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

            lock (_device.ComputeQueue.LockObject)
            {
                vkQueueSubmit2(_device.ComputeQueue.Handle, 1, &submitInfo, context.Fence).DebugCheckResult();
            }
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
