// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.Utilities.MarshalUtilities;
using static Alimer.Graphics.Native.AlimerGPUApi;

namespace Alimer.Graphics.Native;

internal unsafe class NativeGraphicsAdapter : GraphicsAdapter
{
    public NativeGraphicsAdapter(NativeGraphicsManager manager, GPUAdapter handle)
        : base(manager)
    {
        Handle = handle;
        agpuAdapterGetInfo(handle, out GPUAdapterInfo info);

        DeviceName = GetUtf8Span(in info.deviceName[0], GPU_MAX_ADAPTER_NAME_SIZE).GetString() ?? string.Empty;
        VendorId = info.vendorID;
        DeviceId = info.deviceID;
        Type = info.adapterType;
    }

    public GPUAdapter Handle { get; }
    public NativeGraphicsManager NativeManager => (NativeGraphicsManager)Manager;
    public GPUFactory Factory => NativeManager.Handle;

    public override string DeviceName { get; }

    public override uint VendorId { get; }

    public override uint DeviceId { get; }

    public override GraphicsAdapterType Type { get; }

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new NativeGraphicsDevice(this, NativeManager.BackendType, in description);
}
