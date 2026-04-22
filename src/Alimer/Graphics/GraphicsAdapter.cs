// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Defines a physical GPU adapter.
/// </summary>
public abstract class GraphicsAdapter(GraphicsManager manager)
{
    /// <summary>
    /// Gets the manager associated with this adapter.
    /// </summary>
    public GraphicsManager Manager { get; } = manager;

    /// <summary>
    /// Gets the name of the device.
    /// </summary>
    public abstract string DeviceName { get; }

    /// <summary>
    /// Gets the vendor id of the adapter.
    /// </summary>
    public abstract uint VendorId { get; }

    /// <summary>
    /// Gets the device id of the adapter.
    /// </summary>
    public abstract uint DeviceId { get; }

    /// <summary>
    /// Gets the type of the adapter.
    /// </summary>
    public abstract GraphicsAdapterType Type { get; }

    public GraphicsAdapterVendor Vendor
    {
        get
        {
            return VendorId switch
            {
                (uint)KnownGPUAdapterVendor.NVIDIA => GraphicsAdapterVendor.NVIDIA,
                (uint)KnownGPUAdapterVendor.AMD => GraphicsAdapterVendor.AMD,
                (uint)KnownGPUAdapterVendor.INTEL => GraphicsAdapterVendor.Intel,
                (uint)KnownGPUAdapterVendor.ARM => GraphicsAdapterVendor.ARM,
                (uint)KnownGPUAdapterVendor.QUALCOMM => GraphicsAdapterVendor.Qualcomm,
                (uint)KnownGPUAdapterVendor.IMGTECH => GraphicsAdapterVendor.ImgTech,
                (uint)KnownGPUAdapterVendor.MSFT => GraphicsAdapterVendor.MSFT,
                (uint)KnownGPUAdapterVendor.APPLE => GraphicsAdapterVendor.Apple,
                (uint)KnownGPUAdapterVendor.MESA => GraphicsAdapterVendor.Mesa,
                (uint)KnownGPUAdapterVendor.BROADCOM => GraphicsAdapterVendor.Broadcom,
                _ => GraphicsAdapterVendor.Unknown
            };
        }
    }

    /// <summary>
    /// Creates a new <see cref="GraphicsDevice"/> with the default options.
    /// </summary>
    /// <returns></returns>
    public GraphicsDevice CreateDevice() => CreateDevice(new GraphicsDeviceDescription());

    /// <summary>
    /// Creates a new <see cref="GraphicsDevice"/> with the specified options.
    /// </summary>
    /// <param name="description">The graphics device description.</param>
    /// <returns>The graphics device associated with this adapter.</returns>
    public GraphicsDevice CreateDevice(in GraphicsDeviceDescription description)
    {
        return CreateDeviceCore(description);
    }

    protected abstract GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description);
}
