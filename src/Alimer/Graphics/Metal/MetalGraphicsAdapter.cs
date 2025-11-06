// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsAdapter : GraphicsAdapter
{
    public readonly MTLDevice Device;

    public MetalGraphicsAdapter(MetalGraphicsManager manager, MTLDevice device)
        : base(manager)
    {
        Device = device;
    }

    public override string DeviceName => throw new NotImplementedException();

    public override uint VendorId => throw new NotImplementedException();

    public override uint DeviceId => throw new NotImplementedException();

    public override GraphicsAdapterType Type => throw new NotImplementedException();

    public override GraphicsDeviceLimits Limits => throw new NotImplementedException();

    public override bool QueryFeatureSupport(Feature feature) => throw new NotImplementedException();
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) => throw new NotImplementedException();
    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => throw new NotImplementedException();
}
