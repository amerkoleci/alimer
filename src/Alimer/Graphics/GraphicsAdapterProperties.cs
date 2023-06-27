// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics.VGPU;

namespace Alimer.Graphics;

public readonly struct GraphicsAdapterProperties
{
    public readonly uint VendorId { get; init; }
    public readonly uint DeviceId { get; init; }
    public readonly string AdapterName { get; init; }
    public readonly string DriverDescription { get; init; }
    public readonly GraphicsAdapterType AdapterType { get; init; }

    internal static unsafe GraphicsAdapterProperties FromVGPU(in VGPUAdapterProperties native)
    {
        return new()
        {
            VendorId = native.vendorId,
            DeviceId = native.deviceId,
            AdapterName = new(native.name),
            DriverDescription = new(native.driverDescription),
            AdapterType = native.adapterType.FromVGPU()
        };
    }
}
