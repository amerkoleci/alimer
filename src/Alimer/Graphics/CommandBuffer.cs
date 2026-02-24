// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Graphics;

public abstract class CommandBuffer
{
    protected bool _encoderActive;
    protected bool _hasLabel;


    protected CommandBuffer()
    {
    }

    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    protected virtual void Reset(uint frameIndex)
    {
        _hasLabel = false;
        _encoderActive = false;
        //_currentShadingRate = ShadingRate.Invalid;
    }

    public abstract void PushDebugGroup(Utf8ReadOnlyString groupLabel);
    public abstract void PopDebugGroup();
    public abstract void InsertDebugMarker(Utf8ReadOnlyString debugLabel);

    public IDisposable PushScopedDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
    }

    public ComputePassEncoder BeginComputePass() => BeginComputePass(new ComputePassDescriptor());

    public ComputePassEncoder BeginComputePass(in ComputePassDescriptor descriptor)
    {
        if (_encoderActive)
        {
            throw new GraphicsException("CommandEncoder already active");
        }

        ComputePassEncoder computePassEncoder = BeginComputePassCore(in descriptor);
        _encoderActive = true;
        return computePassEncoder;
    }

    public RenderPassEncoder BeginRenderPass(in RenderPassDescriptor renderPass)
    {
        if (_encoderActive)
        {
            throw new GraphicsException("CommandEncoder already active");
        }

        RenderPassEncoder renderPassEncoder = BeginRenderPassCore(in renderPass);
        _encoderActive = true;
        return renderPassEncoder;
    }

    protected abstract ComputePassEncoder BeginComputePassCore(in ComputePassDescriptor descriptor);
    protected abstract RenderPassEncoder BeginRenderPassCore(in RenderPassDescriptor descriptor);

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

            BufferDescriptor bufferDescriptor = new()
            {
                Size = MathUtilities.AlignUp((bufferSize + sizeInBytes) * 2, allocator.Alignment),
                Usage = BufferUsage.Vertex | BufferUsage.Index | BufferUsage.Constant | BufferUsage.ShaderRead,
                MemoryType = MemoryType.Upload,
                Label = "Frame Allocator Buffer"
            };
            //if (Device.QueryFeatureSupport(Feature.RayTracing))
            //{
            //    bufferDescriptor.Usage |= BufferUsage.RayTracing;
            //}

            allocator.Buffer = Device.CreateBuffer(in bufferDescriptor);
            allocator.Offset = 0;
        }

        GPUAllocation allocation = new(allocator.Buffer!, allocator.Offset);

        // Offset allocator
        allocator.Offset += MathUtilities.AlignUp(sizeInBytes, allocator.Alignment);

        Debug.Assert(allocation.IsValid());
        return allocation;
    }

    public void CopyBufferToBuffer(GraphicsBuffer sourceBuffer, GraphicsBuffer destinationBuffer)
    {
        CopyBufferToBufferCore(sourceBuffer, destinationBuffer);
    }

    public void CopyBufferToBuffer(GraphicsBuffer sourceBuffer, ulong sourceOffset, GraphicsBuffer destinationBuffer, ulong destinationOffset, ulong size)
    {
        CopyBufferToBufferCore(sourceBuffer, sourceOffset, destinationBuffer, destinationOffset, size);
    }

    protected abstract void CopyBufferToBufferCore(GraphicsBuffer sourceBuffer, GraphicsBuffer destinationBuffer);
    protected abstract void CopyBufferToBufferCore(GraphicsBuffer sourceBuffer, ulong sourceOffset, GraphicsBuffer destinationBuffer, ulong destinationOffset, ulong size);

    public abstract void Present(SwapChain swapChain);

    #region Nested
    readonly struct ScopedDebugGroup(CommandBuffer commandBuffer) : IDisposable
    {
        private readonly CommandBuffer _commandBuffer = commandBuffer;

        public void Dispose() => _commandBuffer.PopDebugGroup();
    }
    #endregion
}
