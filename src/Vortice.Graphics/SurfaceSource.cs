// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;

namespace Vortice.Graphics
{
    /// <summary>
    /// Defines a platform specific surface used for <see cref="GraphicsSurface"/> creation.
    /// </summary>
    public abstract class SurfaceSource
    {
        protected SurfaceSource()
        {

        }

        public abstract SurfaceSourceType Type { get; }

        /// <summary>
        /// Creates a new <see cref="SurfaceSource"/> for a Win32 window.
        /// </summary>
        /// <param name="hinstance">The Win32 instance handle.</param>
        /// <param name="hwnd">The Win32 window handle.</param>
        /// <returns>A new <see cref="SurfaceSource"/> which can be used to create a <see cref="SwapChainSurface"/> for the given Win32 window.
        /// </returns>
        public static SurfaceSource CreateWin32(IntPtr hinstance, IntPtr hwnd) => new SurfaceSourceWin32(hinstance, hwnd);
    }

    internal class SurfaceSourceWin32 : SurfaceSource
    {
        public IntPtr Hinstance { get; }
        public IntPtr Hwnd { get; }

        public SurfaceSourceWin32(IntPtr hinstance, IntPtr hwnd)
        {
            Hinstance = hinstance;
            Hwnd = hwnd;
        }

        /// <inheritdoc />
        public override SurfaceSourceType Type => SurfaceSourceType.Win32;
    }
}
