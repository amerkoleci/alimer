// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Graphics.Metal.MetalApi;

namespace Alimer.Graphics.Metal;

internal class MetalGraphicsAdapter : GraphicsAdapter
{
    public readonly MTLDevice Device;
    private readonly string _deviceName;
    private readonly uint _deviceId;

    public MetalGraphicsAdapter(MetalGraphicsManager manager, MTLDevice device)
        : base(manager)
    {
        Device = device;
        _deviceName = device.Name;
        _deviceId = unchecked((uint)device.RegistryId);
        // Environment.OSVersion used [[NSProcessInfo processInfo] operatingSystemVersionString]
        Version osVersion = Environment.OSVersion.Version;
        string systemName;
        if (OperatingSystem.IsIOS())
        {
            systemName = "iOS";
        }
        else if(OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            systemName = "macOS";
        }
        else if (OperatingSystem.IsTvOS())
        {
            systemName = "tvOS";
        }
        else if (OperatingSystem.IsWatchOS())
        {
            systemName = "watchOS";
        }
        else
        {
            throw new PlatformNotSupportedException("Unsupported Apple platform.");
        }

        string driverDescription = "Metal driver on " + systemName + " " + osVersion;

        Type = device.HasUnifiedMemory ? GraphicsAdapterType.IntegratedGpu : GraphicsAdapterType.DiscreteGpu;
    }

    public override string DeviceName => _deviceName;

    public override uint VendorId => (uint)KnownGPUAdapterVendor.APPLE;

    public override uint DeviceId => _deviceId;

    public override GraphicsAdapterType Type { get; }

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new MetalGraphicsDevice(this, in description);
}
