// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Diagnostics.Contracts;
using System.Runtime.CompilerServices;
using TerraFX.Interop;
using static Vortice.Graphics.D3D12.D3D12Utils;

namespace Vortice.Graphics.D3D12
{
    internal static class ID3D12DeviceExtensions
    {
        [Pure]
        public static unsafe TFeature CheckFeatureSupport<TFeature>(this ref ID3D12Device2 d3D12Device, D3D12_FEATURE d3D12Feature)
            where TFeature : unmanaged
        {
            TFeature feature;

            ThrowIfFailed(d3D12Device.CheckFeatureSupport(d3D12Feature, &feature, (uint)sizeof(TFeature)));

            return feature;
        }
    }
}
