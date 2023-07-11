// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using static SDL2.SDL.SDL_EventType;
using static SDL2.SDL.SDL_WindowFlags;
using static SDL2.SDL;
using System.Runtime.InteropServices;

namespace Alimer;

internal unsafe class SDLWindow : AppView
{
    private readonly SDLPlatform _platform;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly nint _sdlWindowHandle;
    public readonly uint Id;

    public SDLWindow(SDLPlatform platform)
    {
        _platform = platform;

        SDL_WindowFlags flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

        _title = "Alimer";
#if SDL3
        _sdlWindowHandle = SDL_CreateWindow(_title, 1200, 800, flags);
        SDL_SetWindowPosition(Handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#else
        _sdlWindowHandle = SDL_CreateWindow(_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, flags);
#endif
        Id = SDL_GetWindowID(_sdlWindowHandle);
        SDL_GetWindowSizeInPixels(_sdlWindowHandle, out int width, out int height);
        _clientSize = new Size(width, height);

        // Native handle
        SDL_SysWMinfo wmInfo = default;
#if SDL3
        SDL_GetWindowWMInfo(_sdlWindowHandle, &wmInfo);
#else
        SDL_GetVersion(out wmInfo.version);
        SDL_GetWindowWMInfo(_sdlWindowHandle, ref wmInfo);
#endif

        // Window handle is selected per subsystem as defined at:
        // https://wiki.libsdl.org/SDL_SysWMinfo
        switch (wmInfo.subsystem)
        {
            case SDL_SYSWM_TYPE.SDL_SYSWM_WINDOWS:
                Kind = SwapChainSurfaceType.Win32;
                ContextHandle = GetModuleHandleW(null);
                Handle = wmInfo.info.win.window;
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_WINRT:
                Kind = SwapChainSurfaceType.CoreWindow;
                Handle = wmInfo.info.winrt.window;
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_X11:
                Kind = SwapChainSurfaceType.Xlib;
                ContextHandle = wmInfo.info.x11.display;
                Handle = wmInfo.info.x11.window;
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_COCOA:
                Kind = SwapChainSurfaceType.MetalLayer;
                Handle = wmInfo.info.cocoa.window;
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_WAYLAND:
                Kind = SwapChainSurfaceType.Wayland;
                ContextHandle = wmInfo.info.wl.display;
                Handle = wmInfo.info.wl.surface;
                break;

            case SDL_SYSWM_TYPE.SDL_SYSWM_ANDROID:
                Kind = SwapChainSurfaceType.Android;
                Handle = wmInfo.info.android.window;
                break;

            default:
                break;
        }
    }

    /// <inheritdoc />
    public override bool IsMinimized => _minimized;

    /// <inheritdoc />
    public override SizeF ClientSize => _clientSize;

    /// <inheritdoc />
    public override SwapChainSurfaceType Kind { get; }

    /// <inheritdoc />
    public override nint ContextHandle { get; }

    /// <inheritdoc />
    public override nint Handle { get; }

    [DllImport("SDL2", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SDL_GetWindowSizeInPixels(IntPtr window, out int width, out int height);

    public void Show()
    {
        SDL_ShowWindow(_sdlWindowHandle);
    }

    protected override void SetTitle(string title)
    {
        SDL_SetWindowTitle(_sdlWindowHandle, title);
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

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    private static extern nint GetModuleHandleW(ushort* lpModuleName);
}
