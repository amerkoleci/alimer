// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/winnt.h in the Windows SDK for Windows 10.0.22621.0
// Original source is Copyright © Microsoft. All rights reserved.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#pragma warning disable CS0649

namespace TerraFX.Interop.Windows;

[StructLayout(LayoutKind.Explicit)]
internal partial struct ULARGE_INTEGER
{
    [FieldOffset(0)]
    [NativeTypeName("__AnonymousRecord_winnt_L895_C5")]
    public _Anonymous_e__Struct Anonymous;

    [FieldOffset(0)]
    [NativeTypeName("__AnonymousRecord_winnt_L899_C5")]
    public _u_e__Struct u;

    [FieldOffset(0)]
    [NativeTypeName("ULONGLONG")]
    public ulong QuadPart;

    [UnscopedRef]
    public ref uint LowPart
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return ref Anonymous.LowPart;
        }
    }

    [UnscopedRef]
    public ref uint HighPart
    {
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        get
        {
            return ref Anonymous.HighPart;
        }
    }

    public partial struct _Anonymous_e__Struct
    {
        [NativeTypeName("DWORD")]
        public uint LowPart;

        [NativeTypeName("DWORD")]
        public uint HighPart;
    }

    public partial struct _u_e__Struct
    {
        [NativeTypeName("DWORD")]
        public uint LowPart;

        [NativeTypeName("DWORD")]
        public uint HighPart;
    }
}
