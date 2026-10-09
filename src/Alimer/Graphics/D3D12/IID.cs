// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Vortice.Win32.Graphics;

internal static partial class IID
{
    public static ref readonly Guid IID_ISwapChainPanelNative
    {
        get
        {
            ReadOnlySpan<byte> data = [
                0xD2, 0x19, 0x2F, 0xF9,
                0xDE, 0x3A,
                0xA6, 0x45,
                0xA2,
                0x0C,
                0xF6,
                0xF1,
                0xEA,
                0x90,
                0x55,
                0x4B
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

    public static ref readonly Guid IID_ID3D12VideoDevice
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            ReadOnlySpan<byte> data = [
                0x07, 0x28, 0x05, 0x1F,
                0x46, 0x0B,
                0xCC, 0x4A,
                0x8A,
                0x89,
                0x36,
                0x4F,
                0x79,
                0x37,
                0x18,
                0xA4
            ];

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }
}
