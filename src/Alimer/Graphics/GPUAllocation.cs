// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public unsafe ref struct GPUAllocation
{
    public GPUAllocation(GraphicsBuffer buffer, ulong offset)
    {
        ArgumentNullException.ThrowIfNull(buffer, nameof(buffer));

        Buffer = buffer;
        Offset = offset;
        Data = (byte*)buffer.GetMappedData() + offset;
    }

    public GraphicsBuffer? Buffer { get; }
    public ulong Offset { get; }
    public void* Data { get; } 

    public readonly bool IsValid() => Data is not null && Buffer is not null;
}

public class GPULinearAllocator
{
    public GraphicsBuffer? Buffer;
    public ulong Offset;
    public ulong Alignment;

    public void Reset()
    {
        Offset = 0;
    }
}
