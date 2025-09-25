// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_QUEUE_FLAGS;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandQueue : CommandQueue, IDisposable
{
    private readonly QueueType _queueType;
    private readonly ComPtr<ID3D12CommandQueue> _handle;
    private readonly ComPtr<ID3D12Fence> _fence;
    private ulong _nextFenceValue;
    private ulong _lastCompletedFenceValue;
    private readonly object _fenceMutex = new();
    private readonly object _eventMutex = new();

    private uint _commandBufferCount = 0;
    private readonly List<D3D12CommandBuffer> _commandBuffers = [];
    private readonly List<D3D12SwapChain> _presentSwapChains = [];

    public D3D12CommandQueue(D3D12GraphicsDevice device, QueueType queueType)
    {
        D3DDevice = device;
        _queueType = queueType;
        CommandListType = queueType.ToD3D12();

        _nextFenceValue = (ulong)CommandListType << 56 | 1;
        _lastCompletedFenceValue = (ulong)CommandListType << 56;

        D3D12_COMMAND_QUEUE_DESC queueDesc = new()
        {
            Type = CommandListType,
            Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            NodeMask = 0
        };
        HRESULT hr = device.Handle->CreateCommandQueue(&queueDesc, __uuidof<ID3D12CommandQueue>(), _handle.GetVoidAddressOf());
        if (hr.FAILED)
        {
            throw new GraphicsException("D3D12: Failed to create CommandQueue");
        }

        _fence = device.Handle->CreateFence(true);
        _fence.Get()->Signal(_lastCompletedFenceValue);

        _handle.Get()->SetName($"{queueType}Queue");
        _fence.Get()->SetName($"{queueType}Queue - Fence");
    }


    /// <inheritdoc />
    public override GraphicsDevice Device => D3DDevice;

    /// <inheritdoc />
    public override QueueType QueueType => _queueType;

    public D3D12GraphicsDevice D3DDevice { get; }
    public D3D12_COMMAND_LIST_TYPE CommandListType { get; }
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

        commandBuffer.Begin(D3DDevice.FrameIndex, label);
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
                    D3DDevice.OnDeviceRemoved();
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
                D3D12Texture swapChainTexture = (D3D12Texture)swapChain.GetCurrentTexture()!;
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
            _fence.Get()->SetEventOnCompletion(fenceValue, HANDLE.NULL);
            _lastCompletedFenceValue = fenceValue;
        }
    }

    public void WaitIdle()
    {
        WaitForFence(IncrementFence());
    }
}
