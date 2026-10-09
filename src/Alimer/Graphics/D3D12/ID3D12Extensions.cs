// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Win32;
using Vortice.Win32.Graphics;
using static Alimer.Utilities.UnsafeUtilities;
using static Vortice.Win32.Apis;
using static Vortice.Win32.Graphics.D3D;
using static Vortice.Win32.Graphics.D3D12;

namespace Alimer.Graphics.D3D12;

internal static unsafe partial class ID3D12Extensions
{
    public static void SetDxgiName<TDXGIObject>(ref this TDXGIObject self, ReadOnlySpan<char> name)
        where TDXGIObject : unmanaged, IDXGIObject.Interface
    {
        if (name.IsEmpty)
        {
            _ = self.SetPrivateData(AsPointer(in WKPDID_D3DDebugObjectName), 0u, null);
        }
        else
        {
            fixed (char* pName = name)
            {
                _ = self.SetPrivateData(AsPointer(in WKPDID_D3DDebugObjectName), (uint)name.Length, (ushort*)pName);
            }
        }
    }

    /// <summary>
    /// Creates a new <see cref="ID3D12Fence"/> for a given device.
    /// </summary>
    /// <param name="d3D12Device">The target <see cref="ID3D12Device"/> to use to create the fence.</param>
    /// <returns>A pointer to the newly allocated <see cref="ID3D12Fence"/> instance.</returns>
    /// <exception cref="Exception">Thrown when the creation of the command queue fails.</exception>
    public static ComPtr<ID3D12Fence> CreateFence<TD3D12Device>(this ref TD3D12Device d3D12Device)
        where TD3D12Device : unmanaged, ID3D12Device.Interface
    {
        using ComPtr<ID3D12Fence> d3D12Fence = default;

        d3D12Device.CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            __uuidof<ID3D12Fence>(),
            (void**)d3D12Fence.GetAddressOf()
            ).ThrowIfFailed();

        return d3D12Fence.Move();
    }

    public static ComPtr<ID3D12CommandSignature> CreateCommandSignature<TD3D12Device>(this ref TD3D12Device d3D12Device, D3D12_COMMAND_SIGNATURE_DESC* desc)
        where TD3D12Device : unmanaged, ID3D12Device.Interface
    {
        using ComPtr<ID3D12CommandSignature> result = default;

        d3D12Device.CreateCommandSignature(desc, null, __uuidof<ID3D12CommandSignature>(), (void**)result.GetAddressOf()).ThrowIfFailed();

        return result.Move();
    }
}
