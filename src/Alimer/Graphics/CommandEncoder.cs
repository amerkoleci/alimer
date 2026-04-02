// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Alimer.Graphics.Constants;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Graphics;

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
        SetBindGroup(groupIndex, bindGroup, []);
    }

    public void SetBindGroup(int groupIndex, BindGroup bindGroup, Span<uint> dynamicBufferOffsets)
    {
        ArgumentOutOfRangeException.ThrowIfLessThan(groupIndex, 0, nameof(groupIndex));
        ArgumentNullException.ThrowIfNull(bindGroup, nameof(bindGroup));

        SetBindGroupCore(groupIndex, bindGroup, dynamicBufferOffsets);
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

    protected abstract void SetBindGroupCore(int groupIndex, BindGroup bindGroup, Span<uint> dynamicBufferOffsets);
    protected abstract void SetPushConstantsCore(void* data, uint size, uint offset);

    #region Validation
    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectBuffer(GpuBuffer indirectBuffer)
    {
        if ((indirectBuffer.Usage & GpuBufferUsage.Indirect) == 0)
        {
            throw new GraphicsException($"{nameof(indirectBuffer)} parameter must have been created with BufferUsage.Indirect.");
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
