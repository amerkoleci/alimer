// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.Null;

internal class NullSampler : Sampler
{
    public NullSampler(NullGraphicsDevice device, in SamplerDescription descriptor)
        : base(device, descriptor)
    {
        
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="NullSampler" /> class.
    /// </summary>
    ~NullSampler() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
