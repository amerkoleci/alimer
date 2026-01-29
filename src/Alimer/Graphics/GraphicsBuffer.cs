// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Defines a Graphics buffer.
/// </summary>
public abstract unsafe class GraphicsBuffer : GraphicsObject
{
    protected GraphicsBuffer(in BufferDescriptor descriptor)
        : base(descriptor.Label)
    {
        Size = descriptor.Size;
        Usage = descriptor.Usage;
        MemoryType = descriptor.MemoryType;
    }

    /// <summary>
    /// The total size, in bytes, of the buffer.
    /// </summary>
    public ulong Size { get; }

    /// <summary>
    /// A bitmask indicating this buffer usage.
    /// </summary>
    public BufferUsage Usage { get; }

    /// <summary>
    /// Gets the memory type of this buffer.
    /// </summary>
    public MemoryType MemoryType { get; }

    /// <summary>
    /// Gets the GPU virtual address associated with this resource.
    /// </summary>
    public abstract GPUAddress GpuAddress { get; }

    public BufferStates CurrentState { get; internal set; }

    public void SetData<T>(in T data, int offsetInBytes = 0)
        where T : unmanaged
    {
        // TODO: Copy command buffer
        Guard.IsTrue(MemoryType == MemoryType.Upload);

        fixed (T* dataPtr = &data)
        {
            SetDataUnsafe(dataPtr, offsetInBytes);
        }
    }

    public void SetData<T>(Span<T> data, int offsetInBytes = 0)
        where T : unmanaged
    {
        Guard.IsTrue(MemoryType == MemoryType.Upload);

        fixed (T* dataPtr = data)
        {
            SetDataUnsafe(dataPtr, offsetInBytes);
        }
    }

    public T GetData<T>(int offsetInBytes = 0)
        where T : unmanaged
    {
        T data = new();
        GetData(ref data, offsetInBytes);

        return data;
    }

    public void GetData<T>(ref T data, int offsetInBytes = 0)
        where T : unmanaged
    {
        Guard.IsTrue(MemoryType != MemoryType.Private);

        fixed (T* destPtr = &data)
        {
            GetDataUnsafe(destPtr, offsetInBytes);
        }
    }

    public void GetData<T>(Span<T> destination, int offsetInBytes = 0) where T : unmanaged
    {
        fixed (T* destinationPtr = destination)
        {
            GetDataUnsafe(destinationPtr, offsetInBytes);
        }
    }

    protected abstract void SetDataUnsafe(void* dataPtr, int offsetInBytes);
    protected abstract void GetDataUnsafe(void* destPtr, int offsetInBytes);
}
