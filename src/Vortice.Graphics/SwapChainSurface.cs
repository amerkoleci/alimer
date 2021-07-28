// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Drawing;

namespace Vortice.Graphics
{
    /// <summary>
    /// Defines a platform specific surface used for <see cref="SwapChain"/> creation.
    /// </summary>
    public abstract class SwapChainSurface
    {
        protected SwapChainSurface()
        {

        }

        /// <summary>
        /// Creates a new <see cref="SwapChainSurface"/> for a Win32 window.
        /// </summary>
        /// <param name="hinstance">The Win32 instance handle.</param>
        /// <param name="hwnd">The Win32 window handle.</param>
        /// <returns>A new <see cref="SwapChainSurface"/> which can be used to create a <see cref="SwapChainSurface"/> for the given Win32 window.
        /// </returns>
        public static SwapChainSurface CreateWin32(IntPtr hinstance, IntPtr hwnd) => new SwapChainSurfaceWin32(hinstance, hwnd);
    }

    internal class SwapChainSurfaceWin32 : SwapChainSurface
    {
        public IntPtr Hinstance { get; }
        public IntPtr Hwnd { get; }

        public SwapChainSurfaceWin32(IntPtr hinstance, IntPtr hwnd)
        {
            Hinstance = hinstance;
            Hwnd = hwnd;
        }
    }
}
