// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Text;
using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsAdapter : GraphicsAdapter
{
    private readonly GraphicsDeviceLimits _limits;

    public NativeGraphicsAdapter(NativeGraphicsManager manager, GPUAdapter handle)
        : base(manager)
    {
        Handle = handle;
        agpuAdapterGetInfo(handle, out GPUAdapterInfo info);
        agpuAdapterGetLimits(handle, out GPULimits limits);

        DeviceName = new Utf8String(info.deviceName).ToString();
        VendorId = info.vendorID;
        DeviceId = info.deviceID;
        Type = info.adapterType;

        // TODO: Align GraphicsDeviceLimits with GPULimits
        _limits = new GraphicsDeviceLimits
        {
            MaxTextureDimension1D = limits.maxTextureDimension1D,
            MaxTextureDimension2D = limits.maxTextureDimension2D,
            MaxTextureDimension3D = limits.maxTextureDimension3D,
            MaxTextureDimensionCube = limits.maxTextureDimensionCube,
            MaxTextureArrayLayers = limits.maxTextureArrayLayers,
            MinConstantBufferOffsetAlignment = limits.minConstantBufferOffsetAlignment,
        };
    }

    public GPUAdapter Handle { get; }

    public override string DeviceName { get; }

    public override uint VendorId { get; }

    public override uint DeviceId { get; }

    public override GraphicsAdapterType Type { get; }

    public override GraphicsDeviceLimits Limits => throw new NotImplementedException();

    public override bool QueryFeatureSupport(Feature feature) => throw new NotImplementedException();
    public override PixelFormatSupport QueryPixelFormatSupport(PixelFormat format) => throw new NotImplementedException();
    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new NativeGraphicsDevice(this, description);
}
