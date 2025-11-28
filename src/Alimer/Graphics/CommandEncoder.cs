// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.Constants;

namespace Alimer.Graphics;

public abstract unsafe class CommandEncoder
{
    protected bool _hasLabel;

    protected CommandEncoder()
    {
    }

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
        Guard.IsGreaterThanOrEqualTo(groupIndex, 0, nameof(groupIndex));
        Guard.IsNotNull(bindGroup, nameof(bindGroup));

        // TODO: Use GraphicsAdaterLimits.MaxBindGroups
        Guard.IsLessThan(groupIndex, MaxBindGroups, nameof(groupIndex));

        SetBindGroupCore(groupIndex, bindGroup);
    }

    public void SetPushConstants<T>(uint pushConstantIndex, T data)
         where T : unmanaged
    {
        SetPushConstantsCore(pushConstantIndex, &data, sizeof(T));
    }

    public void SetPushConstants<T>(uint pushConstantIndex, ReadOnlySpan<T> data)
        where T : unmanaged
    {
        int sizeInBytes = data.Length * sizeof(T);

        fixed (void* ptr = data)
            SetPushConstantsCore(pushConstantIndex, ptr, sizeInBytes);
    }

    protected abstract void SetBindGroupCore(int groupIndex, BindGroup bindGroup);
    protected abstract void SetPushConstantsCore(uint pushConstantIndex, void* data, int size);

    #region Validation
    [Conditional("VALIDATE_USAGE")]
    protected internal static void ValidateIndirectBuffer(GraphicsBuffer indirectBuffer)
    {
        if ((indirectBuffer.Usage & BufferUsage.Indirect) == 0)
        {
            throw new GraphicsException($"{nameof(indirectBuffer)} parameter must have been created with BufferUsage.Indirect.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected internal static void ValidateIndirectOffset(ulong offset)
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
