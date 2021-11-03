// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using Vortice.Graphics;
using static SDL2.SDL;
using static SDL2.SDL.SDL_WindowFlags;

namespace Vortice
{
    internal class SDL2GameView : GameView
    {
        private readonly IntPtr _window;

        public SDL2GameView()
        {
            SDL_WindowFlags flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

            _window = SDL_CreateWindow("Vortice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, flags);

            SDL_GetWindowSize(_window, out int width, out int height);
            ClientSize = new Size(width, height);

            // Native handle
            var wmInfo = new SDL_SysWMinfo();
            SDL_GetWindowWMInfo(_window, ref wmInfo);

            // Window handle is selected per subsystem as defined at:
            // https://wiki.libsdl.org/SDL_SysWMinfo
            switch (wmInfo.subsystem)
            {
                case SDL_SYSWM_TYPE.SDL_SYSWM_WINDOWS:
                    Source = SwapChainSource.CreateWin32(
                        wmInfo.info.win.hinstance,
                        wmInfo.info.win.window
                        );
                    break;

                case SDL_SYSWM_TYPE.SDL_SYSWM_X11:
                    //return wmInfo.info.x11.window;
                    break;

                case SDL_SYSWM_TYPE.SDL_SYSWM_COCOA:
                    //return wmInfo.info.cocoa.window;
                    break;

                case SDL_SYSWM_TYPE.SDL_SYSWM_UIKIT:
                    //return wmInfo.info.uikit.window;
                    break;

                case SDL_SYSWM_TYPE.SDL_SYSWM_WAYLAND:
                    //return wmInfo.info.wl.shell_surface;
                    break;

                case SDL_SYSWM_TYPE.SDL_SYSWM_ANDROID:
                    //return wmInfo.info.android.window;
                    break;

                default:
                    break;
            }

            SDL_ShowWindow(_window);
        }

        /// <inheritdoc />
        public override Size ClientSize { get; }

        /// <inheritdoc />
        public override SwapChainSource Source { get; }

        private void OnControlClientSizeChanged(object? sender, EventArgs e)
        {
            OnSizeChanged();
        }
    }
}
