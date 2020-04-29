// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer.Graphics
{
    public abstract class Surface
    {
        public static Surface CreateWin32(IntPtr hInstance, IntPtr hwnd) => new Win32Surface(hInstance, hwnd);
    }

    internal class Win32Surface : Surface
    {
        public Win32Surface(IntPtr hInstance, IntPtr hwnd)
        {
            HInstance = hInstance;
            HWnd = hwnd;
        }


        public IntPtr HInstance { get; }
        public IntPtr HWnd { get; }
    }
}
