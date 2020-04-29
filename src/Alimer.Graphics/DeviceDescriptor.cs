// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    public readonly struct DeviceDescriptor
    {
        public DeviceDescriptor(BackendType preferredBackend = BackendType.Default, GPUPowerPreference powerPreference = GPUPowerPreference.Default, Surface? compatibleSurface = default)
        {
            PreferredBackend = preferredBackend;
            PowerPreference = powerPreference;
            CompatibleSurface = compatibleSurface;
        }

        public BackendType PreferredBackend { get; }
        public GPUPowerPreference PowerPreference { get; }
        public Surface? CompatibleSurface { get; }
    }
}
