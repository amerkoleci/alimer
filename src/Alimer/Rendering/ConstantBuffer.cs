// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

/// <summary>
/// Shader constant buffer interface.
/// </summary>
public sealed unsafe class ConstantBuffer<T> : DisposableObject
     where T : unmanaged
{
    public readonly uint SizeInBytes;
    public readonly GraphicsBuffer Buffer;

    public ConstantBuffer(GraphicsDevice device, MemoryType memoryType = MemoryType.Upload, string? label = default)
    {
        Guard.IsNotNull(device, nameof(device));

        //SizeInBytes = MathHelper.AlignUp((uint)sizeof(T), 16);
        SizeInBytes = (uint)sizeof(T);
        BufferDescriptor descriptor = new(SizeInBytes, BufferUsage.Constant, memoryType, label);
        Buffer = device.CreateBuffer(in descriptor);
    }

    // <summary>
    /// Finalizes an instance of the <see cref="ConstantBuffer" /> class.
    /// </summary>
    ~ConstantBuffer() => Dispose(disposing: false);

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Buffer.Dispose();
        }
    }

    public void SetData(T data, int offsetInBytes = 0)
    {
        Buffer.SetData(data, offsetInBytes);
    }
}
