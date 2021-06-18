// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public struct GraphicsDeviceCaps
    {
        public GraphicsBackend BackendType { get; set; }

        public GPUVendorId VendorId { get; set; }
        public uint AdapterId { get; set; }
        public GPUAdapterType AdapterType { get; set; }
        public string AdapterName { get; set; }
        public GraphicsDeviceFeatures Features;
        public GraphicsDeviceLimits Limits;
    }
}
