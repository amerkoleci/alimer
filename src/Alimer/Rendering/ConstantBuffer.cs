// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Rendering;

/// <summary>
/// Shader constant buffer interface.
/// </summary>
public sealed class ConstantBuffer<T> : DisposableObject
     where T : unmanaged
{
    public readonly uint SizeInBytes;
    public readonly GraphicsBuffer Handle;

    public ConstantBuffer(GraphicsDevice device, MemoryType memoryType = MemoryType.Upload, string? label = default)
    {
        ArgumentNullException.ThrowIfNull(device, nameof(device));

        uint minConstantBufferOffsetAlignment = device.Limits.MinConstantBufferOffsetAlignment;
        uint typeSize = SizeOf<T>();
        SizeInBytes = MathUtilities.AlignUp(typeSize, minConstantBufferOffsetAlignment);
        BufferDescriptor descriptor = new(SizeInBytes, BufferUsage.Constant, memoryType, label);
        Handle = device.CreateBuffer(in descriptor);
    }

    /// <summary>Finalizes an instance of the <see cref="ConstantBuffer" /> class.</summary>
    ~ConstantBuffer() => Dispose(disposing: false);

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Handle.Dispose();
        }
    }

    public void SetData(T data, uint offsetInBytes = 0)
    {
        Handle.SetData(data, offsetInBytes);
    }
}
