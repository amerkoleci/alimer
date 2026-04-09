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

    public void DispatchIndirect(GPUBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DispatchIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void CopyBufferToBuffer(GPUBuffer sourceBuffer, GPUBuffer destinationBuffer)
    {
        CopyBufferToBufferCore(sourceBuffer, destinationBuffer);
    }

    public void CopyBufferToBuffer(GPUBuffer sourceBuffer, ulong sourceOffset, GPUBuffer destinationBuffer, ulong destinationOffset, ulong size)
    {
        CopyBufferToBufferCore(sourceBuffer, sourceOffset, destinationBuffer, destinationOffset, size);
    }

    public void UploadBufferData<T>(GPUBuffer buffer, ulong offset, ReadOnlySpan<T> data)
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

    public void UploadBufferData(GPUBuffer buffer, ulong offset, void* sourceData, ulong size = 0)
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

    protected abstract void CopyBufferToBufferCore(GPUBuffer sourceBuffer, GPUBuffer destinationBuffer);
    protected abstract void CopyBufferToBufferCore(GPUBuffer sourceBuffer, ulong sourceOffset, GPUBuffer destinationBuffer, ulong destinationOffset, ulong size);

    protected abstract void SetPipelineCore(ComputePipeline pipeline);
    protected abstract void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchIndirectCore(GPUBuffer indirectBuffer, ulong indirectBufferOffset);
}
