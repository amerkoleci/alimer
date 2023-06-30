// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandQueue : IDisposable
{
    public readonly VulkanGraphicsDevice Device;
    public readonly QueueType QueueType;
    public readonly VkQueue Handle;
    public readonly object LockObject = new();
    private readonly VkSemaphore _semaphore = VkSemaphore.Null;
    private readonly VkFence[] _frameFences;

    private uint _commandBufferCount = 0;
    private readonly List<VulkanCommandBuffer> _commandBuffers = new();
    private readonly List<VkCommandBuffer> _submitCommandBuffers = new();

    private readonly List<VulkanSwapChain> _presentSwapChains = new();
    private readonly List<VkSemaphore> _submitSignalSemaphores = new();

    public VulkanCommandQueue(VulkanGraphicsDevice device, QueueType queueType)
    {
        Device = device;
        QueueType = queueType;

        uint queueFamilyIndex = device.GetQueueFamily(queueType);
        uint queueIndex = device.GetQueueIndex(queueType);
        vkGetDeviceQueue(device.Handle, queueFamilyIndex, queueIndex, out Handle);

        VkSemaphoreTypeCreateInfo timelineCreateInfo = new()
        {
            pNext = null,
            semaphoreType = VkSemaphoreType.Timeline,
            initialValue = 0
        };

        VkSemaphoreCreateInfo createInfo = new()
        {
            pNext = &timelineCreateInfo,
            flags = 0
        };

        vkCreateSemaphore(device.Handle, &createInfo, null, out _semaphore).DebugCheckResult();

        _frameFences = new VkFence[MaxFramesInFlight];
        for (int frameIndex = 0; frameIndex < MaxFramesInFlight; ++frameIndex)
        {
            vkCreateFence(device.Handle, out _frameFences[frameIndex]);
        }
    }

    public VkFence FrameFence => _frameFences[Device.FrameIndex];

    public void Dispose()
    {
        WaitIdle();
        vkDestroySemaphore(Device.Handle, _semaphore);

        for (int frameIndex = 0; frameIndex < _frameFences.Length; ++frameIndex)
        {
            vkDestroyFence(Device.Handle, _frameFences[frameIndex]);
        }

        foreach (VulkanCommandBuffer commandBuffer in _commandBuffers)
        {
            commandBuffer.Destroy();
        }
        _commandBuffers.Clear();
    }

    public void FinishFrame()
    {
        _commandBufferCount = 0;
        _submitCommandBuffers.Clear();
        _presentSwapChains.Clear();
    }

    public void WaitIdle()
    {
        vkQueueWaitIdle(Handle);
    }

    public void Commit(VulkanCommandBuffer vulkanCommandBuffer, VkCommandBuffer commandBuffer)
    {
        foreach (VulkanSwapChain swapChain in _presentSwapChains)
        {
            VulkanTexture swapChainTexture = swapChain.CurrentTexture;
            vulkanCommandBuffer.TextureBarrier(swapChainTexture, ResourceStates.Present);
        }

        vkEndCommandBuffer(commandBuffer).DebugCheckResult();

        _submitCommandBuffers.Add(commandBuffer);
    }

    public void QueuePresent(VulkanSwapChain swapChain)
    {
        _presentSwapChains.Add(swapChain);
        _submitSignalSemaphores.Add(swapChain.ReleaseSemaphore);

        if (Device.PhysicalDeviceFeatures1_3.synchronization2 == true)
        {
        }
        else
        {

        }
    }

    public void Submit(VkFence fence)
    {
        if (Handle.IsNull)
            return;

        lock (LockObject)
        {
            if (Device.PhysicalDeviceFeatures1_3.synchronization2 == true)
            {
                uint waitSemaphoreInfoCount = (uint)_presentSwapChains.Count;
                uint signalSemaphoreInfoCount = (uint)_presentSwapChains.Count;
                VkSemaphoreSubmitInfo* waitSemaphoreInfos = stackalloc VkSemaphoreSubmitInfo[_presentSwapChains.Count];
                VkSemaphoreSubmitInfo* signalSemaphoreInfos = stackalloc VkSemaphoreSubmitInfo[_presentSwapChains.Count];

                for (int i = 0; i < _presentSwapChains.Count; i++)
                {
                    waitSemaphoreInfos[i] = new()
                    {
                        semaphore = _presentSwapChains[i].AcquireSemaphore,
                        value = 0, // not a timeline semaphore
                        stageMask = VkPipelineStageFlags2.ColorAttachmentOutput
                    };

                    signalSemaphoreInfos[i] = new()
                    {
                        semaphore = _presentSwapChains[i].ReleaseSemaphore,
                        value = 0, // not a timeline semaphore
                    };
                }

                uint commandBufferInfoCount = (uint)_submitCommandBuffers.Count;
                VkCommandBufferSubmitInfo* commandBufferInfos = stackalloc VkCommandBufferSubmitInfo[_submitCommandBuffers.Count];
                for (int i = 0; i < _submitCommandBuffers.Count; i++)
                {
                    commandBufferInfos[i] = new()
                    {
                        commandBuffer = _submitCommandBuffers[i]
                    };
                }

                VkSubmitInfo2 submitInfo = new()
                {
                    waitSemaphoreInfoCount = waitSemaphoreInfoCount,
                    pWaitSemaphoreInfos = waitSemaphoreInfos,
                    commandBufferInfoCount = commandBufferInfoCount,
                    pCommandBufferInfos = commandBufferInfos,
                    signalSemaphoreInfoCount = signalSemaphoreInfoCount,
                    pSignalSemaphoreInfos = signalSemaphoreInfos
                };

                vkQueueSubmit2(Handle, 1, &submitInfo, fence).DebugCheckResult();
            }
            else
            {

            }

            if (_presentSwapChains.Count > 0)
            {
                VkSemaphore* pWaitSemaphores = stackalloc VkSemaphore[_presentSwapChains.Count];
                VkSwapchainKHR* pSwapchains = stackalloc VkSwapchainKHR[_presentSwapChains.Count];
                uint* pImageIndices = stackalloc uint[_presentSwapChains.Count];

                for (int i = 0; i < _presentSwapChains.Count; i++)
                {
                    pWaitSemaphores[i] = _submitSignalSemaphores[i];
                    pSwapchains[i] = _presentSwapChains[i].Handle;
                    pImageIndices[i] = _presentSwapChains[i].AcquiredImageIndex;
                }

                VkPresentInfoKHR presentInfo = new()
                {
                    waitSemaphoreCount = (uint)_submitSignalSemaphores.Count,
                    pWaitSemaphores = pWaitSemaphores,
                    swapchainCount = (uint)_presentSwapChains.Count,
                    pSwapchains = pSwapchains,
                    pImageIndices = pImageIndices
                };

                VkResult result = vkQueuePresentKHR(Handle, &presentInfo);
                if (result != VkResult.Success)
                {
                    // Handle outdated error in present
                    if (result == VkResult.SuboptimalKHR || result == VkResult.ErrorOutOfDateKHR)
                    {
                        //for (auto & swapchain : swapchain_updates)
                        //{
                        //    auto internal_state = to_internal(&swapchain);
                        //    bool success = CreateSwapChainInternal(internal_state, device->physicalDevice, device->device, device->allocationhandler);
                        //    assert(success);
                        //}
                    }
                    else
                    {
                        Debug.Assert(false);
                    }
                }
            }

            //swapchain_updates.clear();
            _presentSwapChains.Clear();
            //submit_waitSemaphoreInfos.clear();
            _submitSignalSemaphores.Clear();
            //submit_signalSemaphoreInfos.clear();
            //submit_cmds.clear();
        }
    }

    public CommandBuffer BeginCommandBuffer(string? label = null)
    {
        uint index = _commandBufferCount++;
        VulkanCommandBuffer commandBuffer;
        if (index >= _commandBuffers.Count)
        {
            commandBuffer = new VulkanCommandBuffer(this);
            _commandBuffers.Add(commandBuffer);
        }
        else
        {
            commandBuffer = _commandBuffers[_commandBuffers.Count - 1];
        }

        commandBuffer.Begin(Device.FrameIndex, label);
        return commandBuffer;
    }
}
