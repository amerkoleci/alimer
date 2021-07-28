// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_FLAGS;
using static TerraFX.Interop.D3D12_COMMAND_QUEUE_PRIORITY;
using static TerraFX.Interop.Windows;
using static Vortice.Graphics.D3D12.Utils;

namespace Vortice.Graphics.D3D12
{
    public unsafe class QueueD3D12
    {
        private ComPtr<ID3D12CommandQueue> _handle;

        public QueueD3D12(GraphicsDeviceD3D12 device, CommandQueueType type)
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = new D3D12_COMMAND_QUEUE_DESC()
            {
                Type = type.ToD3D12(),
                Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                NodeMask = 1
            };

            device.NativeDevice->CreateCommandQueue(&queueDesc, __uuidof<ID3D12CommandQueue>(), _handle.GetVoidAddressOf()).Assert();

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
