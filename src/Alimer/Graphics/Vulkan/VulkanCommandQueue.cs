// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

namespace Alimer.Graphics.Vulkan;

internal unsafe class VulkanCommandQueue : CommandQueue, IDisposable
{
    public readonly VkQueue Handle;
    public readonly object LockObject = new();
    private readonly VkSemaphore _semaphore = VkSemaphore.Null;
    private readonly VkFence[] _frameFences;

    private uint _commandBufferCount = 0;
    private readonly List<VulkanCommandBuffer> _commandBuffers = [];
    private readonly List<VkCommandBuffer> _submitCommandBuffers = [];

    private readonly List<VulkanSwapChain> _presentSwapChains = [];
    private readonly List<VkSemaphore> _submitSignalSemaphores = [];

    public VulkanCommandQueue(VulkanGraphicsDevice device, CommandQueueType queueType)
    {
        VkDevice = device;
        QueueType = queueType;

        uint queueFamilyIndex = device.GetQueueFamily(queueType);
        uint queueIndex = device.GetQueueIndex(queueType);
        device.DeviceApi.vkGetDeviceQueue(device.Handle, queueFamilyIndex, queueIndex, out Handle);

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

        device.DeviceApi.vkCreateSemaphore(device.Handle, &createInfo, null, out _semaphore).CheckResult();

        _frameFences = new VkFence[device.MaxFramesInFlight];
        for (int i = 0; i < device.MaxFramesInFlight; ++i)
        {
            device.DeviceApi.vkCreateFence(device.Handle, out _frameFences[i]);
        }
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => VkDevice;

    /// <inheritdoc />
    public override CommandQueueType QueueType { get; }

    public VulkanGraphicsDevice VkDevice { get; }
    public VkFence FrameFence => _frameFences[Device.FrameIndex];

    public void Dispose()
    {
        WaitIdle();
        VkDevice.DeviceApi.vkDestroySemaphore(VkDevice.Handle, _semaphore);

        for (int i = 0; i < _frameFences.Length; ++i)
        {
            VkDevice.DeviceApi.vkDestroyFence(VkDevice.Handle, _frameFences[i]);
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

    public override void Execute(IEnumerable<CommandBuffer> commandBuffers, bool waitForCompletion = false)
    {
        foreach (VulkanCommandBuffer commandBuffer in commandBuffers)
        {
            VkCommandBuffer handle = commandBuffer.End();
            _submitCommandBuffers.Add(handle);
        }
    }

    /// <inheritdoc />
    public override void WaitIdle()
    {
        VkResult result = VkDevice.DeviceApi.vkQueueWaitIdle(Handle);
        if (result != VK_SUCCESS)
        {
            throw new GraphicsException("Vulkan: Failed to wait for Vulkan queue idle");
        }
    }

    public void QueuePresent(VulkanSwapChain swapChain)
    {
        _presentSwapChains.Add(swapChain);
        _submitSignalSemaphores.Add(swapChain.ReleaseSemaphore);
    }

    public void Submit(VkFence fence)
    {
        if (Handle.IsNull)
            return;

        lock (LockObject)
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

                // Advance surface frame index
                _presentSwapChains[i].AdvanceFrame();
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

            VkDevice.DeviceApi.vkQueueSubmit2(Handle, 1, &submitInfo, fence).CheckResult();

            if (_presentSwapChains.Count > 0)
            {
                VkSemaphore* pWaitSemaphores = stackalloc VkSemaphore[_presentSwapChains.Count];
                VkSwapchainKHR* pSwapchains = stackalloc VkSwapchainKHR[_presentSwapChains.Count];
                uint* pImageIndices = stackalloc uint[_presentSwapChains.Count];

                for (int i = 0; i < _presentSwapChains.Count; i++)
                {
                    pWaitSemaphores[i] = _submitSignalSemaphores[i];
                    pSwapchains[i] = _presentSwapChains[i].Handle;
                    pImageIndices[i] = _presentSwapChains[i].ImageIndex;
                }

                VkPresentInfoKHR presentInfo = new()
                {
                    waitSemaphoreCount = (uint)_submitSignalSemaphores.Count,
                    pWaitSemaphores = pWaitSemaphores,
                    swapchainCount = (uint)_presentSwapChains.Count,
                    pSwapchains = pSwapchains,
                    pImageIndices = pImageIndices
                };

                VkResult result = VkDevice.DeviceApi.vkQueuePresentKHR(Handle, &presentInfo);
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

    public CommandBuffer AcquireCommandBuffer(Utf8ReadOnlyString label = default)
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
