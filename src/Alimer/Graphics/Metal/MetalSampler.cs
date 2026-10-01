// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Constants;

namespace Alimer.Graphics.Metal;

internal sealed class MetalSampler : Sampler
{
    private readonly MetalGraphicsDevice _device;

    public MetalSampler(MetalGraphicsDevice device, in SamplerDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
    }

    public override GraphicsDevice Device => _device;

    public override int BindlessIndex => InvalidBindlessIndex;

    protected internal override void Destroy()
    {
    }
}
