// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.Contracts;
using TerraFX.Interop.DirectX;

namespace Vortice.Graphics;

/// <summary>
/// A <see langword="class"/> with extensions for the <see cref="ID3D12Device2"/> type.
/// </summary>
internal static unsafe class D3D12Extensions
{
    /// <summary>
    /// Checks the feature support of a given type for a given device.
    /// </summary>
    /// <typeparam name="TFeature">The type of feature support data to retrieve.</typeparam>
    /// <param name="d3d12Device">The target <see cref="ID3D12Device"/> to use to check features for.</param>
    /// <param name="d3D12Feature">The type of features to check.</param>
    /// <returns>A <see typeparamref="TFeature"/> value with the features data.</returns>
    [Pure]
    public static unsafe TFeature CheckFeatureSupport<TFeature>(this ref ID3D12Device2 d3d12Device, D3D12_FEATURE d3D12Feature)
        where TFeature : unmanaged
    {
        TFeature feature = default;
        d3d12Device.CheckFeatureSupport(d3D12Feature, &feature, (uint)sizeof(TFeature)).Assert();

        return feature;
    }

    public static void SetName(this ref ID3D12Device2 d3d12Device, string name)
    {
        fixed (char* p = name)
        {
            d3d12Device.SetName((ushort*)p).Assert();
        }
    }

    public static void SetName(this ref ID3D12CommandQueue queue, string name)
    {
        fixed (char* p = name)
        {
            queue.SetName((ushort*)p).Assert();
        }
    }
}
