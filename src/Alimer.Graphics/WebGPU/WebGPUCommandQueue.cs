// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUCommandQueue : IDisposable
{
    public readonly WebGPUGraphicsDevice Device;
    public readonly QueueType QueueType;
    private uint _commandBufferCount = 0;
    private readonly List<WebGPUCommandBuffer> _commandBuffers = new();
    private readonly List<WGPUCommandBuffer> _submitCommandBuffers = new();

    public WebGPUCommandQueue(WebGPUGraphicsDevice device, QueueType queueType)
    {
        Device = device;
        QueueType = queueType;

        Handle = wgpuDeviceGetQueue(device.Handle);
    }

    public WGPUQueue Handle { get; }

    public void Dispose()
    {
        foreach (WebGPUCommandBuffer commandBuffer in _commandBuffers)
        {
            commandBuffer.Destroy();
        }
        _commandBuffers.Clear();
    }

    public void FinishFrame()
    {
        _commandBufferCount = 0;
        _submitCommandBuffers.Clear();
        //_presentSwapChains.Clear();
    }

    public RenderContext BeginCommandContext(string? label = null)
    {
        uint index = _commandBufferCount++;
        WebGPUCommandBuffer commandBuffer;
        if (index >= _commandBuffers.Count)
        {
            commandBuffer = new WebGPUCommandBuffer(this);
            _commandBuffers.Add(commandBuffer);
        }
        else
        {
            commandBuffer = _commandBuffers[_commandBuffers.Count - 1];
        }

        commandBuffer.Begin(Device.FrameIndex, label);
        return commandBuffer;
    }

    public void Commit(WebGPUCommandBuffer commandBuffer, WGPUCommandEncoder commandEncoder)
    {
        //foreach (VulkanSwapChain swapChain in _presentSwapChains)
        //{
        //    VulkanTexture swapChainTexture = swapChain.CurrentTexture;
        //    vulkanCommandBuffer.TextureBarrier(swapChainTexture, ResourceStates.Present);
        //}

        WGPUCommandBufferDescriptor cmdBufferDescriptor = new()
        {
            nextInChain = null
        };

        WGPUCommandBuffer commandBufferHandle = wgpuCommandEncoderFinish(commandEncoder, &cmdBufferDescriptor);
        _submitCommandBuffers.Add(commandBufferHandle);
    }
}
