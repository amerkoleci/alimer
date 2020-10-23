// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public struct GPUDeviceCaps
    {
        public BackendType BackendType;
        public GPUVendorId VendorId;
        public uint AdapterId;
        public GPUAdapterType AdapterType;
        public unsafe fixed byte AdapterName[VGPU.MaxAdapterNameSize];
        public GPUDeviceFeatures Features;
        public GPUDeviceLimits Limits;
    }
}
