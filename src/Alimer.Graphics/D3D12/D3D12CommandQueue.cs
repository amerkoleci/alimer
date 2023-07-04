// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Win32;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandQueue : IDisposable
{
    private readonly D3D12GraphicsDevice _device;
    private readonly ComPtr<ID3D12CommandQueue> _handle;
    private readonly ComPtr<ID3D12Fence> _fence;
    private ulong _nextFenceValue;
    private ulong _lastCompletedFenceValue;
    private readonly object _fenceMutex = new();
    private readonly object _eventMutex = new();

    private uint _commandBufferCount = 0;
    private readonly List<D3D12CommandBuffer> _commandBuffers = new();
    private readonly List<D3D12SwapChain> _presentSwapChains = new();

    public D3D12CommandQueue(D3D12GraphicsDevice device, QueueType type)
    {
        _device = device;
        QueueType = type;
        CommandListType = type.ToD3D12();

        _nextFenceValue = (ulong)CommandListType << 56 | 1;
        _lastCompletedFenceValue = (ulong)CommandListType << 56;

        CommandQueueDescription d3dDesc = new(CommandListType, CommandQueuePriority.Normal);
        HResult hr = device.Handle->CreateCommandQueue(&d3dDesc, __uuidof<ID3D12CommandQueue>(), _handle.GetVoidAddressOf());
        if (hr.Failure)
        {
            throw new GraphicsException("D3D12: Failed to create CommandQueue");
        }

        hr = device.Handle->CreateFence(0, FenceFlags.Shared, __uuidof<ID3D12Fence>(), _fence.GetVoidAddressOf());
        if (hr.Failure)
        {
            throw new GraphicsException("D3D12: Failed to create Fence");
        }
        _fence.Get()->Signal(_lastCompletedFenceValue);

        _handle.Get()->SetName($"{type}Queue");
        _fence.Get()->SetName($"{type}Queue - Fence");
    }

    public D3D12GraphicsDevice Device => _device;
    public QueueType QueueType { get; }
    public CommandListType CommandListType { get; }
    public ID3D12CommandQueue* Handle => _handle;

    /// <inheritdoc />
    public void Dispose()
    {
        foreach (D3D12CommandBuffer commandBuffer in _commandBuffers)
        {
            commandBuffer.Destroy();
        }
        _commandBuffers.Clear();

        _fence.Dispose();
        _handle.Dispose();
    }

    public RenderContext BeginCommandContext(string? label = null)
    {
        uint index = _commandBufferCount++;
        D3D12CommandBuffer commandBuffer;
        if (index >= _commandBuffers.Count)
        {
            commandBuffer = new D3D12CommandBuffer(this);
            _commandBuffers.Add(commandBuffer);
        }
        else
        {
            commandBuffer = _commandBuffers[_commandBuffers.Count - 1];
        }

        commandBuffer.Begin(_device.FrameIndex, label);
        return commandBuffer;
    }

    public void FinishFrame()
    {
        _commandBufferCount = 0;
        _presentSwapChains.Clear();
    }

    public void Submit()
    {
        if (_presentSwapChains.Count > 0)
        {
            foreach (D3D12SwapChain swapChain in _presentSwapChains)
            {
                // If the device was reset we must completely reinitialize the renderer.
                if (!swapChain.Present())
                {
#if DEBUG
                    //char buff[64] = { };
                    //sprintf_s(buff, "Device Lost on Present: Reason code 0x%08X\n",
                    //    static_cast < unsigned int > ((hr == DXGI_ERROR_DEVICE_REMOVED) ? device->GetDeviceRemovedReason() : hr));
                    //OutputDebugStringA(buff);
#endif

                    // Handle device lost
                    _device.OnDeviceRemoved();
                }
            }
        }
    }

    public void QueuePresent(D3D12SwapChain swapChain)
    {
        _presentSwapChains.Add(swapChain);
    }

    public ulong Commit(D3D12CommandBuffer commandBuffer)
    {
        lock (_fenceMutex)
        {
            foreach (D3D12SwapChain swapChain in _presentSwapChains)
            {
                D3D12Texture swapChainTexture = swapChain.CurrentBackBufferTexture;
                commandBuffer.TransitionResource(swapChainTexture, ResourceStates.Present);
            }
            commandBuffer.FlushResourceBarriers();

            ID3D12GraphicsCommandList6* commandList = commandBuffer.CommandList;
            ThrowIfFailed(commandList->Close());

            ID3D12CommandList** commandLists = stackalloc ID3D12CommandList*[1]
            {
                (ID3D12CommandList*)commandList
            };

            // Kickoff the command list
            _handle.Get()->ExecuteCommandLists(1, commandLists);

            // Signal the next fence value (with the GPU)
            _handle.Get()->Signal(_fence.Get(), _nextFenceValue);

            // And increment the fence value.  
            return _nextFenceValue++;
        }
    }

    public ulong IncrementFence()
    {
        lock (_fenceMutex)
        {
            _handle.Get()->Signal(_fence.Get(), _nextFenceValue);
            return _nextFenceValue++;
        }
    }

    public bool IsFenceComplete(ulong fenceValue)
    {
        // Avoid querying the fence value by testing against the last one seen.
        // The max() is to protect against an unlikely race condition that could cause the last
        // completed fence value to regress.
        if (fenceValue > _lastCompletedFenceValue)
        {
            _lastCompletedFenceValue = Math.Max(_lastCompletedFenceValue, _fence.Get()->GetCompletedValue());
        }

        return fenceValue <= _lastCompletedFenceValue;
    }

    public void WaitForFence(ulong fenceValue)
    {
        if (IsFenceComplete(fenceValue))
            return;

        // TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
        // wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
        // the fence can only have one event set on completion, then thread B has to wait for 
        // 100 before it knows 99 is ready.  Maybe insert sequential events?
        lock (_eventMutex)
        {
            _fence.Get()->SetEventOnCompletion(fenceValue, Win32.Handle.Null);
            _lastCompletedFenceValue = fenceValue;
        }
    }

    public void WaitIdle()
    {
        WaitForFence(IncrementFence());
    }
}
