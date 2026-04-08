// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Alimer.Graphics.Constants;
using static Alimer.Utilities.UnsafeUtilities;

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

    public void SetConstantBuffer(uint slot, GpuBuffer buffer, ulong offset = 0)
    {
        ValidateConstantBuffer(buffer);
#if VALIDATE_USAGE
        ArgumentOutOfRangeException.ThrowIfGreaterThanOrEqual(slot, (uint)DynamicContantBufferCount, nameof(slot));
#endif
        SetConstantBufferCore(slot, buffer, offset);
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

    protected abstract void SetBindGroupCore(int groupIndex, BindGroup bindGroup);
    protected abstract void SetConstantBufferCore(uint slot, GpuBuffer buffer, ulong offset);
    protected abstract void SetPushConstantsCore(void* data, uint size, uint offset);

    #region Validation
    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateConstantBuffer(GpuBuffer buffer)
    {
        if ((buffer.Usage & GpuBufferUsage.Constant) == 0)
        {
            throw new GraphicsException($"{nameof(buffer)} parameter must have been created with {GpuBufferUsage.Constant} usage.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectBuffer(GpuBuffer buffer)
    {
        if ((buffer.Usage & GpuBufferUsage.Indirect) == 0)
        {
            throw new GraphicsException($"{nameof(buffer)} parameter must have been created with {GpuBufferUsage.Indirect} usage.");
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
    public GpuBuffer?[] ConstantBuffer { get; } = new GpuBuffer?[DynamicContantBufferCount];
    public ulong[] ConstantBufferOffset { get; } = new ulong[DynamicContantBufferCount];

    public DescriptorBindingTable()
    {

    }
}
