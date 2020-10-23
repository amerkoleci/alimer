// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceCaps
    {
        public readonly BackendType BackendType;
        public readonly GPUVendorId VendorId;
        public readonly uint DeviceId;
        public readonly GPUDeviceFeatures Features;
        public readonly GPUDeviceLimits Limits;
    }
}
