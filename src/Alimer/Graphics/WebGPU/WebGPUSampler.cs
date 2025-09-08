// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUSampler : Sampler
{
    private readonly WebGPUGraphicsDevice _device;

    public WebGPUSampler(WebGPUGraphicsDevice device, in SamplerDescriptor descriptor)
        : base(descriptor)
    {
        _device = device;
        Handle = device.GetOrCreateWGPUSampler(in descriptor);
        wgpuSamplerReference(Handle);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;
    public WGPUSampler Handle { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUSampler" /> class.
    /// </summary>
    ~WebGPUSampler() => Dispose(disposing: false);

    /// <inheitdoc />
    protected internal override void Destroy()
    {
        wgpuSamplerRelease(Handle);
    }

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
        wgpuSamplerSetLabel(Handle, newLabel);
    }
}
