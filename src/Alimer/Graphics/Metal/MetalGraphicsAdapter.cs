// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsAdapter : GraphicsAdapter
{
    public MetalGraphicsAdapter(MetalGraphicsManager manager)
        : base(manager)
    {
    }

    public override string DeviceName => throw new NotImplementedException();

    public override uint VendorId => throw new NotImplementedException();

    public override uint DeviceId => throw new NotImplementedException();

    public override GraphicsAdapterType Type => throw new NotImplementedException();

    public override GraphicsDeviceLimits Limits => throw new NotImplementedException();

    public override bool QueryFeatureSupport(Feature feature) => throw new NotImplementedException();
    public override bool QueryPixelFormatSupport(PixelFormat format) => throw new NotImplementedException();
    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => throw new NotImplementedException();
}
