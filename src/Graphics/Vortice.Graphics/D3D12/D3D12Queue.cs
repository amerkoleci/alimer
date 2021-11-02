// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.D3D12_FENCE_FLAGS;
using System;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12Queue
    {
        private readonly ComPtr<ID3D12CommandQueue> _handle;
        private readonly ComPtr<ID3D12Fence> _fence;
        private readonly HANDLE _fenceEventHandle;
        private ulong _nextFenceValue = 0;
        private ulong _lastCompletedFenceValue = 0;

        public D3D12Queue(D3D12GraphicsDevice device, CommandQueueType type)
        {
            D3D12_COMMAND_QUEUE_DESC d3D12CommandQueueDesc;
            d3D12CommandQueueDesc.Type = type.ToD3D12();
            d3D12CommandQueueDesc.Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            d3D12CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            d3D12CommandQueueDesc.NodeMask = 0;

            device.NativeDevice->CreateCommandQueue(
                &d3D12CommandQueueDesc,
                __uuidof<ID3D12CommandQueue>(),
                _handle.GetVoidAddressOf()).Assert();

            _handle.Get()->SetName($"{type} Command Queue");

            device.NativeDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof<ID3D12Fence>(), _fence.GetVoidAddressOf()).Assert();
            _fenceEventHandle = CreateEventExW(lpEventAttributes: null, lpName: null, dwFlags: 0, dwDesiredAccess: EVENT_MODIFY_STATE | SYNCHRONIZE);
            if (_fenceEventHandle == HANDLE.NULL)
            {
                //ThrowForLastError(nameof(CreateEventW));
            }
        }

        public ID3D12CommandQueue* Handle => _handle;

        /// <inheritdoc />
        public void Dispose()
        {
            CloseHandle(_fenceEventHandle);
            _fence.Dispose();
            _handle.Dispose();
        }

        public ulong Signal()
        {
            //std::lock_guard<std::mutex> LockGuard(m_FenceMutex);
            _handle.Get()->Signal(_fence.Get(), _nextFenceValue);
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

                _fence.Get()->SetEventOnCompletion(fenceValue, _fenceEventHandle);
                WaitForSingleObject(_fenceEventHandle, INFINITE);
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
                _lastCompletedFenceValue = Math.Max(_lastCompletedFenceValue, _fence.Get()->GetCompletedValue());
            }

            return fenceValue <= _lastCompletedFenceValue;
        }
    }
}
