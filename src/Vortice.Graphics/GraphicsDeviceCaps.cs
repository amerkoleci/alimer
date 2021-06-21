// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public readonly struct GraphicsDeviceCaps
    {
        public GraphicsBackend BackendType { get; init; }

        public GPUVendorId VendorId { get; init; }
        public uint AdapterId { get; init; }
        public GPUAdapterType AdapterType { get; init; }
        public string? AdapterName { get; init; }
        public GraphicsDeviceFeatures Features { get; init; }
        public GraphicsDeviceLimits Limits { get; init; }
    }
}
