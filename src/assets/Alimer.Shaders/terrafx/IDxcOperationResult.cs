// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/dxcapi.h in the Windows SDK for Windows 10.0.26100.0
// Original source is Copyright © Microsoft. All rights reserved. Licensed under the University of Illinois Open Source License.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.IID;

namespace TerraFX.Interop.DirectX;

[Guid("CEDB484A-D4E9-445A-B991-CA21CA157DC2")]
[NativeTypeName("struct IDxcOperationResult : IUnknown")]
[NativeInheritance("IUnknown")]
internal unsafe partial struct IDxcOperationResult : IDxcOperationResult.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IDxcOperationResult);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(0)]
    public HRESULT QueryInterface([NativeTypeName("const IID &")] Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, Guid*, void**, int>)(lpVtbl[0]))((IDxcOperationResult*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(1)]
    [return: NativeTypeName("ULONG")]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, uint>)(lpVtbl[1]))((IDxcOperationResult*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(2)]
    [return: NativeTypeName("ULONG")]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, uint>)(lpVtbl[2]))((IDxcOperationResult*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(3)]
    public HRESULT GetStatus(HRESULT* pStatus)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, HRESULT*, int>)(lpVtbl[3]))((IDxcOperationResult*)Unsafe.AsPointer(ref this), pStatus);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(4)]
    public HRESULT GetResult(IDxcBlob** ppResult)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, IDxcBlob**, int>)(lpVtbl[4]))((IDxcOperationResult*)Unsafe.AsPointer(ref this), ppResult);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(5)]
    public HRESULT GetErrorBuffer(IDxcBlobEncoding** ppErrors)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcOperationResult*, IDxcBlobEncoding**, int>)(lpVtbl[5]))((IDxcOperationResult*)Unsafe.AsPointer(ref this), ppErrors);
    }

    public interface Interface : IUnknown.Interface
    {
        [VtblIndex(3)]
        HRESULT GetStatus(HRESULT* pStatus);

        [VtblIndex(4)]
        HRESULT GetResult(IDxcBlob** ppResult);

        [VtblIndex(5)]
        HRESULT GetErrorBuffer(IDxcBlobEncoding** ppErrors);
    }
}
