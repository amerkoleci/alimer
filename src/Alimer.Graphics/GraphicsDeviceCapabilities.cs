// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    public struct GraphicsDeviceCapabilities
    {
        /// The backend type.
        public BackendType Backend;

        /// Selected GPU vendor PCI id.
        public int VendorId;
        public int DeviceId;
        public string AdapterName;
    }
}
