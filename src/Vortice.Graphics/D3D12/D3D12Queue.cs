// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.D3D12Utils;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_PRIORITY;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12Queue
    {
        private readonly ComPtr<ID3D12CommandQueue> _handle;

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
        }

        public ID3D12CommandQueue* Handle => _handle;

        /// <inheritdoc />
        public void Dispose()
        {
            _handle.Dispose();
        }
    }
}
