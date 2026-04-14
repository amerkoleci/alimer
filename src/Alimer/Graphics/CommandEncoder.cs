// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Alimer.Graphics.Constants;
using static Alimer.Utilities.UnsafeUtilities;
using static Alimer.Numerics.MathUtilities;

namespace Alimer.Graphics;

// TODO: Do we expose barriers here?

public abstract unsafe class CommandEncoder
{
    protected bool _hasLabel;

    protected CommandEncoder()
    {
    }

    public abstract GraphicsDevice Device { get; }

    public abstract void EndEncoding();

    public abstract void PushDebugGroup(Utf8ReadOnlyString groupLabel);
    public abstract void PopDebugGroup();
    public abstract void InsertDebugMarker(Utf8ReadOnlyString debugLabel);

    public IDisposable PushScopedDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
    }

    public void SetBindGroup(int groupIndex, BindGroup bindGroup)
    {
        ArgumentOutOfRangeException.ThrowIfLessThan(groupIndex, 0, nameof(groupIndex));
        ArgumentNullException.ThrowIfNull(bindGroup, nameof(bindGroup));

        SetBindGroupCore(groupIndex, bindGroup);
    }

    public void SetConstantBuffer(uint slot, GPUBuffer buffer, ulong offset = 0)
    {
        ValidateConstantBuffer(buffer);
#if VALIDATE_USAGE
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(slot, (uint)DynamicContantBufferCount, nameof(slot));
#endif
        SetConstantBufferCore(slot, buffer, offset);
    }

    public void SetDynamicConstantBuffer<T>(uint slot, in T data)
        where T : unmanaged
    {
        uint sizeInBytes = SizeOf<T>();
        GPUAllocation allocation = Allocate(sizeInBytes, Device.Limits.MinConstantBufferOffsetAlignment);
        fixed (T* pointer = &data)
        {
            new Span<T>(pointer, 1).CopyTo(new(allocation.Data, 1));
        }
        SetConstantBufferCore(slot, allocation.Buffer!, allocation.Offset);
    }

    public void SetDynamicConstantBuffer<T>(uint slot, ReadOnlySpan<T> data)
        where T : unmanaged
    {
        if (data.Length is 0)
        {
            return;
        }

        uint sizeInBytes = (uint)(SizeOf<T>() * data.Length);
        GPUAllocation allocation = Allocate(sizeInBytes, Device.Limits.MinConstantBufferOffsetAlignment);
        data.CopyTo(new(allocation.Data, data.Length));
        SetConstantBufferCore(slot, allocation.Buffer!, allocation.Offset);
    }

    public void SetPushConstants<T>(T data, uint offset = 0)
         where T : unmanaged
    {
        SetPushConstantsCore(&data, SizeOf<T>(), offset);
    }

    public void SetPushConstants<T>(ReadOnlySpan<T> data, uint offset = 0)
        where T : unmanaged
    {
        uint sizeInBytes = (uint)data.Length * SizeOf<T>();

        fixed (void* ptr = data)
            SetPushConstantsCore(ptr, sizeInBytes, offset);
    }

    public void SetPushConstants(void* data, uint size, uint offset = 0)
    {
        SetPushConstantsCore(data, size, offset);
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
            alignment = Device.Limits.MinLinearAllocatorOffsetAlignment;

        ulong bufferSize = (allocator.Buffer is null) ? 0 : allocator.Buffer.Size;
        ulong freeSpace = bufferSize - allocator.Offset;
        if (sizeInBytes > freeSpace)
        {
            allocator.Alignment = alignment;

            // Dispose old buffer
            allocator.Buffer?.Dispose();

            GPUBufferDescriptor bufferDescriptor = new()
            {
                Size = AlignUp((bufferSize + sizeInBytes) * 2, allocator.Alignment),
                Usage = GPUBufferUsage.Vertex | GPUBufferUsage.Index | GPUBufferUsage.Constant | GPUBufferUsage.ShaderRead,
                MemoryType = MemoryType.Upload,
                Label = "Frame Allocator Buffer"
            };
            if (Device.Limits.RayTracingTier != RayTracingTier.NotSupported)
            {
                bufferDescriptor.Usage |= GPUBufferUsage.RayTracing;
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

    protected abstract void SetBindGroupCore(int groupIndex, BindGroup bindGroup);
    protected abstract void SetConstantBufferCore(uint slot, GPUBuffer buffer, ulong offset/*, ulong size = WholeSize*/);
    protected abstract void SetPushConstantsCore(void* data, uint size, uint offset);

    #region Validation
    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateConstantBuffer(GPUBuffer buffer)
    {
        if ((buffer.Usage & GPUBufferUsage.Constant) == 0)
        {
            throw new GraphicsException($"{nameof(buffer)} parameter must have been created with {GPUBufferUsage.Constant} usage.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectBuffer(GPUBuffer buffer)
    {
        if ((buffer.Usage & GPUBufferUsage.Indirect) == 0)
        {
            throw new GraphicsException($"{nameof(buffer)} parameter must have been created with {GPUBufferUsage.Indirect} usage.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectOffset(ulong offset)
    {
        if ((offset % 4) != 0)
        {
            throw new GraphicsException($"{nameof(offset)} must be a multiple of 4.");
        }
    }
    #endregion Validation

    readonly struct ScopedDebugGroup(CommandEncoder encoder) : IDisposable
    {
        private readonly CommandEncoder _encoder = encoder;

        public void Dispose() => _encoder.PopDebugGroup();
    }
}

public readonly struct DescriptorBindingTable
{
    public GPUBuffer?[] ConstantBuffer { get; } = new GPUBuffer?[DynamicContantBufferCount];
    public ulong[] ConstantBufferOffset { get; } = new ulong[DynamicContantBufferCount];

    public DescriptorBindingTable()
    {

    }

    public void Reset()
    {
        for (int i = 0; i < ConstantBuffer.Length; i++)
        {
            ConstantBuffer[i] = null;
            ConstantBufferOffset[i] = 0;
        }
    }
}
