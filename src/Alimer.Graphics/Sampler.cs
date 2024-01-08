// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public abstract class Sampler : GraphicsResource
{
    protected Sampler(in SamplerDescriptor descriptor)
        : base(descriptor.Label)
    {
    }
}
