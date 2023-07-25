// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUSampler : Sampler
{
    private readonly WebGPUGraphicsDevice _device;

    public WebGPUSampler(WebGPUGraphicsDevice device, in SamplerDescription description)
        : base(description)
    {
        _device = device;

        fixed (sbyte* pLabel = description.Label.GetUtf8Span())
        {
            WGPUSamplerDescriptor descriptor = new()
            {
                label = pLabel,
                magFilter = description.MagFilter.ToWebGPU(),
                minFilter = description.MinFilter.ToWebGPU(),
                mipmapFilter = description.MipFilter.ToWebGPU(),
                addressModeU = description.AddressModeU.ToWebGPU(),
                addressModeV = description.AddressModeV.ToWebGPU(),
                addressModeW = description.AddressModeW.ToWebGPU(),
                maxAnisotropy = description.MaxAnisotropy
            };

            Handle = wgpuDeviceCreateSampler(device.Handle, &descriptor);

            if (Handle.IsNull)
            {
                Log.Error("WebGPU: Failed to create sampler.");
                return;
            }
        }
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
