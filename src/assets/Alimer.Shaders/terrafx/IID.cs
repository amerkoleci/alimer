// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/dxcapi.h in the Windows SDK for Windows 10.0.26100.0
// Original source is Copyright © Microsoft. All rights reserved. Licensed under the University of Illinois Open Source License.

using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace TerraFX.Interop.Windows;

internal static partial class IID
{
    public static ref readonly Guid IID_IDxcBlob
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x08, 0xFB, 0xA5, 0x8B,
                0x95, 0x51,
                0xE2, 0x40,
                0xAC,
                0x58,
                0x0D,
                0x98,
                0x9C,
                0x3A,
                0x01,
                0x02
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcUtils
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xCB, 0xC4, 0x05, 0x46,
                0x19, 0x20,
                0x2A, 0x49,
                0xAD,
                0xA4,
                0x65,
                0xF2,
                0x0B,
                0xB7,
                0xD6,
                0x7F
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcBlobEncoding
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x24, 0xD4, 0x41, 0x72,
                0x46, 0x26,
                0x91, 0x41,
                0x97,
                0xC0,
                0x98,
                0xE9,
                0x6E,
                0x42,
                0xFC,
                0x68
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcBlobUtf8
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xC9, 0x36, 0xA6, 0x3D,
                0x71, 0xBA,
                0x24, 0x40,
                0xA3,
                0x01,
                0x30,
                0xCB,
                0xF1,
                0x25,
                0x30,
                0x5B
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcBlobWide
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xAB, 0x4E, 0xF8, 0xA3,
                0xAA, 0x0F,
                0x7E, 0x49,
                0xA3,
                0x9C,
                0xEE,
                0x6E,
                0xD6,
                0x0B,
                0x2D,
                0x84
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcOperationResult
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x4A, 0x48, 0xDB, 0xCE,
                0xE9, 0xD4,
                0x5A, 0x44,
                0xB9,
                0x91,
                0xCA,
                0x21,
                0xCA,
                0x15,
                0x7D,
                0xC2
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcResult
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0xDA, 0x6C, 0x34, 0x58,
                0xE7, 0xDD,
                0x97, 0x44,
                0x94,
                0x61,
                0x6F,
                0x87,
                0xAF,
                0x5E,
                0x06,
                0x59
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcIncludeHandler
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x7D, 0xFC, 0x61, 0x7F,
                0x0D, 0x95,
                0x7F, 0x46,
                0xB3,
                0xE3,
                0x3C,
                0x02,
                0xFB,
                0x49,
                0x18,
                0x7C
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_IDxcCompiler3
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x87, 0x46, 0x8B, 0x22,
                0x6A, 0x5A,
                0x30, 0x47,
                0x90,
                0x0C,
                0x97,
                0x02,
                0xB2,
                0x20,
                0x3F,
                0x54
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }
}
