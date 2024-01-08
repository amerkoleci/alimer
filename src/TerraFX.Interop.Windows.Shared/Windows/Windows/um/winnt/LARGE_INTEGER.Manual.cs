// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from um/winnt.h in the Windows SDK for Windows 10.0.22621.0
// Original source is Copyright © Microsoft. All rights reserved.

using System.Runtime.CompilerServices;

namespace TerraFX.Interop.Windows;

internal partial struct LARGE_INTEGER
{
    public static implicit operator long(LARGE_INTEGER value) => value.QuadPart;

    public static implicit operator LARGE_INTEGER(long value)
    {
        Unsafe.SkipInit(out LARGE_INTEGER result);
        result.QuadPart = value;
        return result;
    }
}
