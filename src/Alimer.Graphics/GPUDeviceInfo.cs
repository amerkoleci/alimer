// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public struct GPUDeviceInfo
    {
        public Bool32 Debug;
        public GPUDevicePreference DevicePreference;
        public SwapchainInfo SwapchainInfo;
    }
}
