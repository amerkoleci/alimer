// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Vortice.Win32.Graphics;

internal static partial class IID
{
    public static ref readonly Guid IID_ISwapChainPanelNativeWinUI
    {
        get
        {
            ReadOnlySpan<byte> data = [
                0xB8, 0xD0, 0xAA, 0x63,
                0x24, 0x7C,
                0xFF, 0x40,
                0x85,
                0xA8,
                0x64,
                0x0D,
                0x94,
                0x4C,
                0xC3,
                0x25
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
