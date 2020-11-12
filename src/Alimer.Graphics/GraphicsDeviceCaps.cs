// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    public struct GraphicsDeviceCaps
    {
        public BackendType BackendType;
        public GPUVendorId VendorId;
        public uint AdapterId;
        public GraphicsAdapterType AdapterType;
        public string AdapterName;
        public GPUDeviceFeatures Features;
        public GPUDeviceLimits Limits;
    }
}
