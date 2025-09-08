// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Alimer.Graphics.D3D;

internal static partial class IID
{
    public static ref readonly Guid IID_ISwapChainPanelNative
    {
        get
        {
            ReadOnlySpan<byte> data = new byte[] {
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
            };

            Debug.Assert(data.Length == Unsafe.SizeOf<Guid>());
            return ref Unsafe.As<byte, Guid>(ref MemoryMarshal.GetReference(data));
        }
    }

}
