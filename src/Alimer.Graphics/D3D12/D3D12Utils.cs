// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Vortice;
using Vortice.Direct3D;
using Vortice.DXGI;
using static Vortice.Direct3D12.D3D12;

namespace Alimer.Graphics.D3D12
{
    internal static class D3D12Utils
    {
        public static IDXGIAdapter1? GetAdapter(IDXGIFactory4 factory, FeatureLevel minFeatureLevel, GraphicsAdapterPreference adapterPreference)
        {
            IDXGIAdapter1? adapter = null;

            IDXGIFactory6? factory6 = factory.QueryInterfaceOrNull<IDXGIFactory6>();
            if (factory6 != null)
            {
                GpuPreference gpuPreference = GpuPreference.HighPerformance;
                if (adapterPreference == GraphicsAdapterPreference.LowPower)
                {
                    gpuPreference = GpuPreference.MinimumPower;
                }

                for (int adapterIndex = 0; factory6.EnumAdapterByGpuPreference(adapterIndex, gpuPreference, out adapter).Success; adapterIndex++)
                {
                    if (adapter == null)
                    {
                        continue;
                    }

                    if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
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

                factory6.Dispose();
            }

            if (adapter == null)
            {
                for (int adapterIndex = 0; factory.EnumAdapters1(adapterIndex, out adapter).Success; adapterIndex++)
                {
                    if (adapter == null)
                    {
                        continue;
                    }

                    if ((adapter.Description1.Flags & AdapterFlags.Software) != 0)
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
