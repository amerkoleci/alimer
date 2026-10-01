// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Metal;

internal sealed class MetalTextureView : TextureView
{
    public MetalTextureView(MetalTexture texture, in TextureViewDescriptor descriptor)
        : base(texture, in descriptor)
    {
    }

    public override int BindlessReadIndex => InvalidBindlessIndex;

    public override int BindlessReadWriteIndex => InvalidBindlessIndex;

    internal override void Destroy()
    {
    }
}
