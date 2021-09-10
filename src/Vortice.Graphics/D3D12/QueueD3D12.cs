// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Direct3D12;

namespace Vortice.Graphics.D3D12
{
    public unsafe class QueueD3D12
    {
        public QueueD3D12(GraphicsDeviceD3D12 device, CommandQueueType type)
        {
            CommandQueueDescription queueDesc = new()
            {
                Type = type.ToD3D12(),
                Priority = (int)CommandQueuePriority.Normal,
                Flags = CommandQueueFlags.None,
                NodeMask = 1
            };

            Handle = device.NativeDevice.CreateCommandQueue<ID3D12CommandQueue>(queueDesc);
            Handle.Name = $"{type} Command Queue";
        }

        public ID3D12CommandQueue Handle { get; }

        /// <inheritdoc />
        public void Dispose()
        {
            Handle.Dispose();
        }
    }
}
