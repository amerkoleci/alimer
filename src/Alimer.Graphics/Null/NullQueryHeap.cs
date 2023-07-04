// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Null;

internal class NullQueryHeap : QueryHeap
{
    public NullQueryHeap(NullGraphicsDevice device, in QueryHeapDescription description)
        : base(description)
    {
        Device = device;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="NullQueryHeap" /> class.
    /// </summary>
    ~NullQueryHeap() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
