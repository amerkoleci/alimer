// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.Direct3D;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;

namespace Alimer.Graphics.D3D12
{
    internal static unsafe class D3D12Utils
    {
        public static IDXGIAdapter1? GetAdapter(IDXGIFactory4 factory, FeatureLevel minFeatureLevel, bool lowPower)
        {
            IDXGIAdapter1? adapter = null;
            using (IDXGIFactory6 dxgiFactory6 = factory.QueryInterfaceOrNull<IDXGIFactory6>())
            {
                GpuPreference gpuPreference = GpuPreference.HighPerformance;
                if (lowPower)
                {
                    gpuPreference = GpuPreference.MinimumPower;
                }

                for (int adapterIndex = 0;
                    ResultCode.NotFound != dxgiFactory6.EnumAdapterByGpuPreference(adapterIndex, gpuPreference, out adapter);
                    adapterIndex++)
                {
                    AdapterDescription1 desc = adapter!.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter.Dispose();
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (IsSupported(adapter, minFeatureLevel))
                    {
                        break;
                    }
                }

                dxgiFactory6.Dispose();
            }

            if (adapter == null)
            {
                for (int adapterIndex = 0;
                    ResultCode.NotFound != factory.EnumAdapters1(adapterIndex, out adapter);
                    adapterIndex++)
                {
                    AdapterDescription1 desc = adapter.Description1;

                    if ((desc.Flags & AdapterFlags.Software) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter.Dispose();
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (IsSupported(adapter, minFeatureLevel))
                    {
                        break;
                    }
                }
            }

            return adapter;
        }
    }
}
