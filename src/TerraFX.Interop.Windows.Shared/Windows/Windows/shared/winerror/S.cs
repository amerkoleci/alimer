// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from shared/winerror.h in the Windows SDK for Windows 10.0.20348.0
// Original source is Copyright © Microsoft. All rights reserved.

namespace TerraFX.Interop.Windows;

internal static partial class S
{
    [NativeTypeName("#define S_OK ((HRESULT)0L)")]
    public const int S_OK = ((int)(0));

    [NativeTypeName("#define S_FALSE ((HRESULT)1L)")]
    public const int S_FALSE = ((int)(1));
}
