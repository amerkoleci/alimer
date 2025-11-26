// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace TerraFX.Interop.Windows;

internal static partial class Windows
{
    public static bool SUCCEEDED(HRESULT hr)
    {
        return hr >= 0;
    }

    public static bool FAILED(HRESULT hr)
    {
        return hr < 0;
    }

    [DoesNotReturn]
    public static void ThrowExternalException(string methodName, int errorCode)
    {
        var message = string.Format("'{0}' failed with an error code of '{1}'", methodName, errorCode);
        throw new ExternalException(message, errorCode);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static void ThrowIfFailed(HRESULT value, [CallerArgumentExpression("value")] string? valueExpression = null)
    {
        if (value.FAILED)
        {
            ThrowExternalException(valueExpression ?? "Method", value);
        }
    }
}
