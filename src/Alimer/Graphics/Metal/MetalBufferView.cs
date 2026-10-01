// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Metal;

internal sealed class MetalBufferView : GraphicsBufferView
{
    public MetalBufferView(MetalBuffer buffer, in GraphicsBufferViewDescriptor descriptor)
        : base(buffer, in descriptor)
    {
    }

    public override int BindlessReadIndex => InvalidBindlessIndex;

    public override int BindlessReadWriteIndex => InvalidBindlessIndex;

    protected internal override void Destroy()
    {
    }
}
