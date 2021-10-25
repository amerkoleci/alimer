// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;

namespace Vortice.Graphics.D3D12
{
    internal unsafe class D3D12GraphicsSurface : GraphicsSurface
    {
        public D3D12GraphicsSurface(D3D12GraphicsDeviceFactory factory, in SurfaceSource source)
            : base(source)
        {
            switch (source.Type)
            {
                case SurfaceSourceType.Win32:
                    SurfaceSourceWin32 win32Source = (SurfaceSourceWin32)source;
                    Guard.IsTrue(IsWindow(win32Source.Hwnd) == TRUE, nameof(source));
                    RECT rect;
                    GetClientRect(win32Source.Hwnd, &rect);

                    Size = new(rect.right - rect.left, rect.bottom - rect.top);
                    break;
            }
        }

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {

            }
        }
    }
}
