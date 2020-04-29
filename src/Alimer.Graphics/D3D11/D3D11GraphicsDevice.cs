// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.DXGI;
using static Vortice.Direct3D11.D3D11;

namespace Alimer.Graphics.D3D11
{
    internal class D3D11GraphicsDevice : GraphicsDevice
    {
        private static readonly FeatureLevel[] s_featureLevels = new[]
        {
            FeatureLevel.Level_12_1,
            FeatureLevel.Level_12_0,
            FeatureLevel.Level_11_1,
            FeatureLevel.Level_11_0
        };

        private readonly GraphicsDeviceCapabilities _capabilities;
        public readonly ID3D11Device Device;
        public readonly FeatureLevel FeatureLevel;
        public readonly ID3D11DeviceContext DeviceContext;

        public D3D11GraphicsDevice(in DeviceDescriptor descriptor)
        {
            // Detect adapter to use.
            IDXGIAdapter1 dxgiAdapter = null;

            if (descriptor.PowerPreference != GPUPowerPreference.Default)
            {
                var factory6 = D3D11Instance.Factory.QueryInterfaceOrNull<IDXGIFactory6>();
                if (factory6 != null)
                {
                    var gpuPreference = GpuPreference.HighPerformance;
                    if (descriptor.PowerPreference == GPUPowerPreference.LowPower)
                    {
                        gpuPreference = GpuPreference.MinimumPower;
                    }

                    var adapters = factory6.EnumAdaptersByGpuPreference<IDXGIAdapter1>(gpuPreference);
                    foreach (var adapter in adapters)
                    {
                        if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
                        {
                            // Don't select the Basic Render Driver adapter.
                            continue;
                        }

                        dxgiAdapter = adapter;
                        dxgiAdapter.AddRef();
                        break;
                    }

                    Utilities.ReleaseIfNotDefault(adapters);
                    factory6.Dispose();
                }
            }

            if (dxgiAdapter == null)
            {
                var dxgiAdapters = D3D11Instance.Factory.EnumAdapters1();
                foreach (var adapter in dxgiAdapters)
                {
                    if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    dxgiAdapter = adapter;
                    dxgiAdapter.AddRef();
                    break;
                }
                Utilities.ReleaseIfNotDefault(dxgiAdapters);
            }

            var creationFlags = DeviceCreationFlags.BgraSupport;
            if (EnableValidation)
            {
                creationFlags |= DeviceCreationFlags.Debug;
            }

            if (D3D11CreateDevice(
                dxgiAdapter,
                DriverType.Unknown,
                creationFlags,
                s_featureLevels,
                out Device, out var FeatureLevel, out DeviceContext).Failure)
            {
                // Remove debug flag not being supported.
                creationFlags &= ~DeviceCreationFlags.Debug;

                var result = D3D11CreateDevice(null, DriverType.Hardware,
                    creationFlags, s_featureLevels,
                    out Device, out FeatureLevel, out DeviceContext);
                if (result.Failure)
                {
                    // This will fail on Win 7 due to lack of 11.1, so re-try again without it
                    D3D11CreateDevice(
                        null,
                        DriverType.Hardware,
                        creationFlags,
                        new[] { FeatureLevel.Level_11_0 },
                        out Device, out FeatureLevel, out DeviceContext).CheckError();
                }
            }


            dxgiAdapter.Release();

            // Init capabilities.
            _capabilities.Backend = BackendType.Direct3D11;
        }

        public override GraphicsDeviceCapabilities Capabilities => _capabilities;

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D11GraphicsDevice" /> class.
        /// </summary>
        ~D3D11GraphicsDevice()
        {
            Dispose(disposing: false);
        }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                DeviceContext.ClearState();
                DeviceContext.Flush();
                DeviceContext.Dispose();
                Device.Dispose();
            }
        }
    }
}
