// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D12;

namespace Vortice.Graphics;

internal unsafe class D3D12Queue
{
    private readonly ID3D12Fence _fence;
    private readonly AutoResetEvent _fenceEvent;
    private ulong _nextFenceValue = 0;
    private ulong _lastCompletedFenceValue = 0;

    public D3D12Queue(D3D12GraphicsDevice device, CommandQueueType type)
    {
        CommandQueueDescription queueDesc = new CommandQueueDescription
        {
            Type = type.ToD3D12(),
            Priority = (int)CommandQueuePriority.Normal,
            Flags = CommandQueueFlags.None,
            NodeMask = 0
        };

        Handle = device.NativeDevice.CreateCommandQueue(queueDesc);
        Handle.Name = $"{type} Command Queue";

        _fence = device.NativeDevice.CreateFence(0);
        _fenceEvent = new AutoResetEvent(false);
    }

    public ID3D12CommandQueue Handle { get; }

    /// <inheritdoc />
    public void Dispose()
    {
        _fenceEvent.Dispose();
        _fence.Dispose();
        Handle.Dispose();
    }

    public ulong Signal()
    {
        //std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
        Handle.Signal(_fence, _nextFenceValue);
        return _nextFenceValue++;
    }

    public void WaitForFence(ulong fenceValue)
    {
        if (IsFenceComplete(fenceValue))
        {
            return;
        }

        {
            //std::lock_guard<std::mutex> LockGuard(m_EventMutex);

            _fence.SetEventOnCompletion(fenceValue, _fenceEvent);
            _fenceEvent.WaitOne();
            _lastCompletedFenceValue = fenceValue;
        }
    }

    public void WaitIdle()
    {
        WaitForFence(Signal());
    }

    public bool IsFenceComplete(ulong fenceValue)
    {
        // Avoid querying the fence value by testing against the last one seen.
        // The max() is to protect against an unlikely race condition that could cause the last
        // completed fence value to regress.
        if (fenceValue > _lastCompletedFenceValue)
        {
            _lastCompletedFenceValue = Math.Max(_lastCompletedFenceValue, _fence.CompletedValue);
        }

        return fenceValue <= _lastCompletedFenceValue;
    }
}
