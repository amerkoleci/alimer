// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Null;

internal class NullBuffer : GraphicsBuffer
{
    public NullBuffer(NullGraphicsDevice device, in BufferDescription descriptor)
        : base(device, descriptor)
    {
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="NullQueryHeap" /> class.
    /// </summary>
    ~NullBuffer() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
