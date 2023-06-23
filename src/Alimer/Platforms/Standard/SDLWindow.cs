// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using static SDL2.SDL.SDL_EventType;
using static SDL2.SDL.SDL_WindowFlags;
using static SDL2.SDL;
using System.Runtime.InteropServices;

namespace Alimer;

internal unsafe class SDLWindow : GameView
{
    private readonly SDLPlatform _platform;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly nint Handle;
    public readonly uint Id;

    public SDLWindow(SDLPlatform platform)
    {
        _platform = platform;

        SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

#if SDL3
        Handle = SDL_CreateWindow("Alimer", 1200, 800, flags);
        SDL_SetWindowPosition(Handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#else
        Handle = SDL_CreateWindow("Alimer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, flags);
#endif
        SDL_ShowWindow(Handle);
        Id = SDL_GetWindowID(Handle);
        SDL_GetWindowSizeInPixels(Handle, out int width, out int height);
        _clientSize = new Size(width, height);

        // Native handle
        SDL_SysWMinfo wmInfo = default;
#if SDL3
        SDL_GetWindowWMInfo(Handle, &wmInfo);
#else
        SDL_GetVersion(out wmInfo.version);
        SDL_GetWindowWMInfo(Handle, ref wmInfo);
#endif

        // Window handle is selected per subsystem as defined at:
        // https://wiki.libsdl.org/SDL_SysWMinfo
        switch (wmInfo.subsystem)
        {
            case SDL_SYSWM_TYPE.SDL_SYSWM_WINDOWS:
                //Surface = SwapChainSurface.CreateWin32(wmInfo.info.win.window);
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_WINRT:
                //Surface = SwapChainSurface.CreateCoreWindow(wmInfo.info.winrt.window);
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
    }

    /// <inheritdoc />
    public override bool IsMinimized => _minimized;

    /// <inheritdoc />
    public override Size ClientSize => _clientSize;

    /// <inheritdoc />
    //public override SwapChainSurface Surface { get; }

    [DllImport("SDL2", CallingConvention = CallingConvention.Cdecl)]
    private static extern void SDL_GetWindowSizeInPixels(IntPtr window, out int width, out int height);

    public void Destroy()
    {
        SDL_DestroyWindow(Handle);
    }

    public void Show()
    {
        SDL_ShowWindow(Handle);
    }

    public void HandleEvent(in SDL_Event evt)
    {
#if TODO
        switch (evt.window.type)
        {
            case SDL_EVENT_WINDOW_MINIMIZED:
                _minimized = true;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            //case SDL_EVENT_WINDOW_CHANGED:
            //    _minimized = false;
            //    _clientSize = new(evt.window.data1, evt.window.data2);
            //    OnSizeChanged();
            //    break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                //DestroySurface(window);
                _platform.WindowClosed(evt.window.windowID);
                SDL_DestroyWindow(Handle);
                break;
        } 
#endif
    }
}
