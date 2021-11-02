// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Runtime.InteropServices;
using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;

namespace Vortice.Audio
{
    internal static unsafe class XAudio2Helpers
    {
        public static void ThrowIfFailed(HRESULT hr)
        {
            if (hr.FAILED)
            {
                ThrowHelper.ThrowWin32Exception(hr);
            }
        }
    }
}
