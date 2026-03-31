// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Defines a GPU buffer.
/// </summary>
public abstract unsafe class GpuBuffer : GraphicsObject, IGraphicsBindableResource
{
    protected GpuBuffer(in BufferDescriptor descriptor)
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
    public abstract GpuAddress GpuAddress { get; }

    /// <summary>
    /// Gets the bindless shader read (SRV) index of the view.
    /// </summary>
    public abstract int BindlessShaderReadIndex { get; }

    /// <summary>
    /// Gets the bindless shader write (UAV) index of the view.
    /// </summary>
    public abstract int BindlessShaderWriteIndex { get; }

    internal BufferStates CurrentState { get; set; }
    internal abstract void* GetMappedData();

    public void SetData<T>(in T source, uint offsetInBytes = 0)
        where T : unmanaged
    {
        // TODO: Copy command buffer
        Debug.Assert(MemoryType == MemoryType.Upload);

        fixed (T* sourcePtr = &source)
        {
            SetDataUnsafe(sourcePtr, offsetInBytes, (uint)sizeof(T));
        }
    }

    public void SetData<T>(Span<T> source, uint offsetInBytes = 0)
        where T : unmanaged
    {
        Debug.Assert(MemoryType == MemoryType.Upload);

        fixed (T* sourcePtr = source)
        {
            SetDataUnsafe(sourcePtr, offsetInBytes, (uint)(source.Length * sizeof(T)));
        }
    }

    public void GetData<T>(ref T destination, uint offsetInBytes = 0)
        where T : unmanaged
    {
        Debug.Assert(MemoryType != MemoryType.Private);

        fixed (T* destinationPtr = &destination)
        {
            GetDataUnsafe(destinationPtr, offsetInBytes, (uint)sizeof(T));
        }
    }

    public void GetData<T>(Span<T> destination, uint offsetInBytes = 0) where T : unmanaged
    {
        fixed (T* destinationPtr = destination)
        {
            GetDataUnsafe(destinationPtr, offsetInBytes, (uint)(destination.Length * sizeof(T)));
        }
    }

    protected abstract void SetDataUnsafe(void* sourcePtr, uint offsetInBytes, uint sizeInBytes);
    protected abstract void GetDataUnsafe(void* destinationPtr, uint offsetInBytes, uint sizeInBytes);
}
