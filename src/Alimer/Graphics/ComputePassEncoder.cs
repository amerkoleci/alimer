// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.InteropServices;
using static Alimer.Utilities.UnsafeUtilities;
using static Alimer.Numerics.MathUtilities;

namespace Alimer.Graphics;

public abstract unsafe class ComputePassEncoder : CommandEncoder
{
    protected ComputePassEncoder()
    {
    }

    protected virtual void Reset()
    {
        _hasLabel = false;
    }

    public void SetPipeline(ComputePipeline pipeline)
    {
        ArgumentNullException.ThrowIfNull(pipeline, nameof(pipeline));

        SetPipelineCore(pipeline);
    }

    public void Dispatch1D(uint threadCountX, uint groupSizeX = 64u)
    {
        Dispatch(DivideByMultiple(threadCountX, groupSizeX), 1u, 1u);
    }

    public void Dispatch2D(uint threadCountX, uint threadCountY, uint groupSizeX = 8u, uint groupSizeY = 8u)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeX),
            1u
        );
    }

    public void Dispatch3D(uint threadCountX, uint threadCountY, uint threadCountZ, uint groupSizeX, uint groupSizeY, uint groupSizeZ)
    {
        Dispatch(
            DivideByMultiple(threadCountX, groupSizeX),
            DivideByMultiple(threadCountY, groupSizeY),
            DivideByMultiple(threadCountZ, groupSizeZ)
        );
    }

    public void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    public void DispatchIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DispatchIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void CopyBufferToBuffer(GraphicsBuffer sourceBuffer, GraphicsBuffer destinationBuffer)
    {
        CopyBufferToBufferCore(sourceBuffer, destinationBuffer);
    }

    public void CopyBufferToBuffer(GraphicsBuffer sourceBuffer, ulong sourceOffset, GraphicsBuffer destinationBuffer, ulong destinationOffset, ulong size)
    {
        CopyBufferToBufferCore(sourceBuffer, sourceOffset, destinationBuffer, destinationOffset, size);
    }
    public GPUAllocation Allocate<T>(ulong alignment = 0)
        where T : unmanaged
    {
        return Allocate(SizeOf<T>(), alignment);
    }

    public GPUAllocation Allocate(ulong sizeInBytes, ulong alignment = 0)
    {
        GPULinearAllocator allocator = Device.FrameAllocator;
        if (alignment == 0)
            alignment = Device.LinearAllocatorAlignment;

        ulong bufferSize = (allocator.Buffer is null) ? 0 : allocator.Buffer.Size;
        ulong freeSpace = bufferSize - allocator.Offset;
        if (sizeInBytes > freeSpace)
        {
            allocator.Alignment = alignment;

            // Dispose old buffer
            allocator.Buffer?.Dispose();

            BufferDescriptor bufferDescriptor = new()
            {
                Size = AlignUp((bufferSize + sizeInBytes) * 2, allocator.Alignment),
                Usage = BufferUsage.Vertex | BufferUsage.Index | BufferUsage.Constant | BufferUsage.ShaderRead,
                MemoryType = MemoryType.Upload,
                Label = "Frame Allocator Buffer"
            };
            if (Device.QueryFeatureSupport(Feature.RayTracing))
            {
                bufferDescriptor.Usage |= BufferUsage.RayTracing;
            }

            allocator.Buffer = Device.CreateBuffer(in bufferDescriptor);
            allocator.Offset = 0;
        }

        GPUAllocation allocation = new(allocator.Buffer!, allocator.Offset);

        // Offset allocator
        allocator.Offset += AlignUp(sizeInBytes, allocator.Alignment);

        Debug.Assert(allocation.IsValid());
        return allocation;
    }

    public void UploadBufferData<T>(GraphicsBuffer buffer, ulong offset, ReadOnlySpan<T> data)
        where T : unmanaged
    {
        if (data.Length is 0)
        {
            return;
        }

        uint sizeInBytes = (uint)(SizeOf<T>() * data.Length);
        GPUAllocation allocation = Allocate(sizeInBytes);
        data.CopyTo(new(allocation.Data, data.Length));

        CopyBufferToBuffer(allocation.Buffer!, allocation.Offset, buffer, offset, sizeInBytes);
    }

    public void UploadBufferData<T>(GraphicsBuffer buffer, ulong offset, void* sourceData, ulong size = 0)
    {
        if (buffer == null || sourceData == null)
            return;

        size = Math.Min(buffer.Size, size);
        if (size == 0)
            return;

        GPUAllocation allocation = Allocate(size);
        NativeMemory.Copy(sourceData, allocation.Data, (nuint)size);
        CopyBufferToBuffer(allocation.Buffer!, allocation.Offset, buffer, offset, size);
    }

    protected abstract void CopyBufferToBufferCore(GraphicsBuffer sourceBuffer, GraphicsBuffer destinationBuffer);
    protected abstract void CopyBufferToBufferCore(GraphicsBuffer sourceBuffer, ulong sourceOffset, GraphicsBuffer destinationBuffer, ulong destinationOffset, ulong size);

    protected abstract void SetPipelineCore(ComputePipeline pipeline);
    protected abstract void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
}
