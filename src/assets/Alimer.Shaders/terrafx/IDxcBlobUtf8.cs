// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/dxcapi.h in the Windows SDK for Windows 10.0.26100.0
// Original source is Copyright © Microsoft. All rights reserved. Licensed under the University of Illinois Open Source License.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.Windows.IID;

#pragma warning disable CS0649

namespace TerraFX.Interop.DirectX;

[Guid("3DA636C9-BA71-4024-A301-30CBF125305B")]
[NativeTypeName("struct IDxcBlobUtf8 : IDxcBlobEncoding")]
[NativeInheritance("IDxcBlobEncoding")]
internal unsafe partial struct IDxcBlobUtf8 : IDxcBlobUtf8.Interface, INativeGuid
{
    static Guid* INativeGuid.NativeGuid => (Guid*)Unsafe.AsPointer(in IID_IDxcBlobUtf8);

    public void** lpVtbl;

    /// <inheritdoc cref="IUnknown.QueryInterface" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(0)]
    public HRESULT QueryInterface([NativeTypeName("const IID &")] Guid* riid, void** ppvObject)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, Guid*, void**, int>)(lpVtbl[0]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this), riid, ppvObject);
    }

    /// <inheritdoc cref="IUnknown.AddRef" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(1)]
    [return: NativeTypeName("ULONG")]
    public uint AddRef()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, uint>)(lpVtbl[1]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IUnknown.Release" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(2)]
    [return: NativeTypeName("ULONG")]
    public uint Release()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, uint>)(lpVtbl[2]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IDxcBlob.GetBufferPointer" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(3)]
    [return: NativeTypeName("LPVOID")]
    public void* GetBufferPointer()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, void*>)(lpVtbl[3]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IDxcBlob.GetBufferSize" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(4)]
    [return: NativeTypeName("SIZE_T")]
    public nuint GetBufferSize()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, nuint>)(lpVtbl[4]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    /// <inheritdoc cref="IDxcBlobEncoding.GetEncoding" />
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(5)]
    public HRESULT GetEncoding(BOOL* pKnown, [NativeTypeName("UINT32 *")] uint* pCodePage)
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, BOOL*, uint*, int>)(lpVtbl[5]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this), pKnown, pCodePage);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(6)]
    [return: NativeTypeName("LPCSTR")]
    public byte* GetStringPointer()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, byte*>)(lpVtbl[6]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    [VtblIndex(7)]
    [return: NativeTypeName("SIZE_T")]
    public nuint GetStringLength()
    {
        return ((delegate* unmanaged[MemberFunction]<IDxcBlobUtf8*, nuint>)(lpVtbl[7]))((IDxcBlobUtf8*)Unsafe.AsPointer(ref this));
    }

    public interface Interface : IDxcBlobEncoding.Interface
    {
        [VtblIndex(6)]
        [return: NativeTypeName("LPCSTR")]
        byte* GetStringPointer();

        [VtblIndex(7)]
        [return: NativeTypeName("SIZE_T")]
        nuint GetStringLength();
    }
}
