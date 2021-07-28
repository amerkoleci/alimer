// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.D3D12_COMMAND_LIST_TYPE;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.Utils;

namespace Vortice.Graphics.D3D12
{
    public unsafe class QueueD3D12
    {
        private ComPtr<ID3D12CommandQueue> _handle;

        public QueueD3D12(GraphicsDeviceD3D12 device, CommandQueueType commandQueue)
        {
            // Create queues
            D3D12_COMMAND_QUEUE_DESC d3D12CommandQueueDesc;

            d3D12CommandQueueDesc.Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            d3D12CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            d3D12CommandQueueDesc.NodeMask = 1;

            switch (commandQueue)
            {
                case CommandQueueType.Graphics:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case CommandQueueType.Compute:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case CommandQueueType.Copy:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
            }

            device.NativeDevice->CreateCommandQueue(
              &d3D12CommandQueueDesc,
              __uuidof<ID3D12CommandQueue>(),
              _handle.GetVoidAddressOf()).Assert();

            switch (commandQueue)
            {
                case CommandQueueType.Graphics:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    break;
                case CommandQueueType.Compute:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
                    break;
                case CommandQueueType.Copy:
                    d3D12CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
                    break;
            }

            _handle.Get()->SetName($"{commandQueue} Command Queue");
        }

        public ID3D12CommandQueue* Handle => _handle;

        /// <inheritdoc />
        public void Dispose()
        {
            _handle.Dispose();
        }
    }
}
