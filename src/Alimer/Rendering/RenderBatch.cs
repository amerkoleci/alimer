// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Rendering;

public sealed unsafe class RenderBatch : DisposableObject
{
    private const uint InitialInstanceCount = 128;

    private const uint InstanceSizeInBytes = 20; // 4x float (16) + 1x float4 (color)

    private GraphicsBuffer? _instanceBuffer;
    private bool _instanceBufferDirty = true;
    private uint _instanceCapacity = 0;

    public RenderBatch(GraphicsDevice device)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        Device = device;
        ResizeInstanceBuffer(InitialInstanceCount);
    }

    public GraphicsDevice Device { get; }

    public void ResizeInstanceBuffer(uint capacity)
    {
        _instanceBuffer?.Dispose();

        _instanceBufferDirty = true;
        _instanceCapacity = capacity;
        _instanceBuffer = ToDispose(Device.CreateBuffer(
            InstanceSizeInBytes * capacity,
            BufferUsage.Vertex,
            MemoryType.Upload,
            label: "Upload Instance Buffer"
            ));
    }
}
