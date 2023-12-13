// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/winnt.h in the Windows SDK for Windows 10.0.20348.0
// Original source is Copyright © Microsoft. All rights reserved.

#pragma warning disable CS0649

namespace TerraFX.Interop.Windows;

internal partial struct LUID
{
    [NativeTypeName("DWORD")]
    public uint LowPart;

    [NativeTypeName("LONG")]
    public int HighPart;
}
