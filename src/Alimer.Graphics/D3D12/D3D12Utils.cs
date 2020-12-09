// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static TerraFX.Interop.DXGI_GPU_PREFERENCE;
using static TerraFX.Interop.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DXGI_FORMAT;
using System;

namespace Alimer.Graphics.D3D12
{
    internal static unsafe class D3D12Utils
    {
        public static void ThrowIfFailed(HRESULT hr)
        {
            if (FAILED(hr))
            {
                Marshal.ThrowExceptionForHR(hr);
            }
        }

        public static IDXGIAdapter1* GetAdapter(IDXGIFactory4* factory, D3D_FEATURE_LEVEL minFeatureLevel, bool lowPower)
        {
            Guid IDXGIAdapter1_iid = IID_IDXGIAdapter1;
            Guid factory6_iid = IID_IDXGIFactory6;
            Guid ID3D12Device_iid = IID_ID3D12Device;

            IDXGIAdapter1* adapter = null;
            using ComPtr<IDXGIFactory6> dxgiFactory6 = null;
            if (SUCCEEDED(factory->QueryInterface(&factory6_iid, (void**)&dxgiFactory6)))
            {
                DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
                if (lowPower)
                {
                    gpuPreference = DXGI_GPU_PREFERENCE_MINIMUM_POWER;
                }

                for (uint adapterIndex = 0;
                    DXGI_ERROR_NOT_FOUND != dxgiFactory6.Get()->EnumAdapterByGpuPreference(adapterIndex, gpuPreference, &IDXGIAdapter1_iid, (void**)&adapter);
                    adapterIndex++)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    ThrowIfFailed(adapter->GetDesc1(&desc));

                    if ((desc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter->Release();
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, minFeatureLevel, &ID3D12Device_iid, null)))
                    {
                        break;
                    }
                }
            }

            if (adapter == null)
            {
                for (uint adapterIndex = 0;
                    DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter);
                    adapterIndex++)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    ThrowIfFailed(adapter->GetDesc1(&desc));

                    if ((desc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                    {
                        // Don't select the Basic Render Driver adapter.
                        adapter->Release();
                        continue;
                    }

                    // Check to see if the adapter supports Direct3D 12.
                    if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, minFeatureLevel, &ID3D12Device_iid, null)))
                    {
                        break;
                    }
                }
            }

            return adapter;
        }
    }
}
