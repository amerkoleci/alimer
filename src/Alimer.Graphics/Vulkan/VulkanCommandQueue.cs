// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandQueue : IDisposable
{
    public readonly VulkanGraphicsDevice Device;
    public readonly CommandQueue QueueType;
    public readonly VkQueue Handle;
    public readonly object LockObject = new();
    private readonly VkSemaphore _semaphore = VkSemaphore.Null;
    private readonly VkFence[] _frameFences;

    private uint _commandBufferCount = 0;
    private List<VulkanCommandBuffer> _commandBuffers = new();

    public VulkanCommandQueue(VulkanGraphicsDevice device, CommandQueue queueType)
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

        vkCreateSemaphore(device.Handle, &createInfo, null, out _semaphore).CheckResult();

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

        foreach(VulkanCommandBuffer commandBuffer in _commandBuffers)
        {
            commandBuffer.Destroy();
        }
        _commandBuffers.Clear();
    }

    public void FinishFrame()
    {
        _commandBufferCount = 0;
    }

    public void WaitIdle()
    {
        vkQueueWaitIdle(Handle);
    }

    public void Submit(VkFence fence)
    {
        if (Handle.IsNull)
            return;

        lock (LockObject)
        {
            VkSubmitInfo2 submitInfo = new()
            {
                sType = VkStructureType.SubmitInfo2
            };
            //submitInfo.waitSemaphoreInfoCount = (uint32_t)submit_waitSemaphoreInfos.size();
            //submitInfo.pWaitSemaphoreInfos = submit_waitSemaphoreInfos.data();
            //submitInfo.commandBufferInfoCount = (uint32_t)submit_cmds.size();
            //submitInfo.pCommandBufferInfos = submit_cmds.data();
            //submitInfo.signalSemaphoreInfoCount = (uint32_t)submit_signalSemaphoreInfos.size();
            //submitInfo.pSignalSemaphoreInfos = submit_signalSemaphoreInfos.data();

            vkQueueSubmit2(Handle, 1, &submitInfo, fence).DebugCheckResult();

            //if (!submitSwapchains.empty())
            //{
            //    VkPresentInfoKHR presentInfo = { };
            //    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            //    presentInfo.waitSemaphoreCount = (uint32_t)submit_signalSemaphores.size();
            //    presentInfo.pWaitSemaphores = submit_signalSemaphores.data();
            //    presentInfo.swapchainCount = (uint32_t)submit_swapchains.size();
            //    presentInfo.pSwapchains = submit_swapchains.data();
            //    presentInfo.pImageIndices = submit_swapChainImageIndices.data();
            //    res = vkQueuePresentKHR(queue, &presentInfo);
            //    if (res != VK_SUCCESS)
            //    {
            //        // Handle outdated error in present:
            //        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
            //        {
            //            for (auto & swapchain : swapchain_updates)
            //            {
            //                auto internal_state = to_internal(&swapchain);
            //                bool success = CreateSwapChainInternal(internal_state, device->physicalDevice, device->device, device->allocationhandler);
            //                assert(success);
            //            }
            //        }
            //        else
            //        {
            //            assert(0);
            //        }
            //    }
            //}

            //swapchain_updates.clear();
            //submit_swapchains.clear();
            //submit_swapChainImageIndices.clear();
            //submit_waitSemaphoreInfos.clear();
            //submit_signalSemaphores.clear();
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
