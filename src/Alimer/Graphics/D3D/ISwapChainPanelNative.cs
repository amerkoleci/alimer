// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using TerraFX.Interop;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.D3D.IID;

#pragma warning disable CS0649

namespace Alimer.Graphics.D3D;

[Guid("F92F19D2-3ADE-45A6-A20C-F6F1EA90554B")]
[SupportedOSPlatform("windows6.3")]
internal unsafe partial struct ISwapChainPanelNative : ISwapChainPanelNative.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(in IID_ISwapChainPanelNative));

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT QueryInterface(Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<ISwapChainPanelNative*, Guid*, void**, int>)(lpVtbl[0]))((ISwapChainPanelNative*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<ISwapChainPanelNative*, uint>)(lpVtbl[1]))((ISwapChainPanelNative*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<ISwapChainPanelNative*, uint>)(lpVtbl[2]))((ISwapChainPanelNative*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public HRESULT SetSwapChain(IDXGISwapChain* swapChain)
    {
        return ((delegate* unmanaged[MemberFunction]<ISwapChainPanelNative*, IDXGISwapChain*, int>)(lpVtbl[3]))((ISwapChainPanelNative*)Unsafe.AsPointer(ref this), swapChain);
    }

    public interface Interface : IUnknown.Interface
    {
        HRESULT SetSwapChain(IDXGISwapChain* swapChain);
    }
}
