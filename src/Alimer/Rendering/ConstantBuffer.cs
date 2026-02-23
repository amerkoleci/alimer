// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Rendering;

/// <summary>
/// Shader constant buffer interface.
/// </summary>
public sealed unsafe class ConstantBuffer<T> : DisposableObject
     where T : unmanaged
{
    public readonly uint SizeInBytes;
    public readonly GraphicsBuffer Handle;

    public ConstantBuffer(GraphicsDevice device, MemoryType memoryType = MemoryType.Upload, string? label = default)
    {
        Guard.IsNotNull(device, nameof(device));

        uint minConstantBufferOffsetAlignment = device.Limits.MinConstantBufferOffsetAlignment;
        uint typeSize = SizeOf<T>();
        SizeInBytes = MathUtilities.AlignUp(typeSize, minConstantBufferOffsetAlignment);
        BufferDescriptor descriptor = new(SizeInBytes, BufferUsage.Constant, memoryType, label);
        Handle = device.CreateBuffer(in descriptor);
    }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        Handle.Dispose();
    }

    public void SetData(T data, uint offsetInBytes = 0)
    {
        Handle.SetData(data, offsetInBytes);
    }
}
