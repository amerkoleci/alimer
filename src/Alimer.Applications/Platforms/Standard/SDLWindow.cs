// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using static SDL2.SDL.SDL_WindowEventID;
using static SDL2.SDL.SDL_WindowFlags;
using static SDL2.SDL;
using System.Runtime.InteropServices;

namespace Alimer;

internal unsafe class SDLWindow : Window
{
    private readonly SDLPlatform _platform;
    private Size _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly nint SDLWindowHandle;
    public readonly uint Id;

    public SDLWindow(SDLPlatform platform, WindowFlags flags)
    {
        _platform = platform;

        SDL_WindowFlags sdlWindowFlags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;

        if ((flags & WindowFlags.Borderless) != 0)
            sdlWindowFlags |= SDL_WINDOW_BORDERLESS;

        if ((flags & WindowFlags.Resizable) != 0)
            sdlWindowFlags |= SDL_WINDOW_RESIZABLE;

        if ((flags & WindowFlags.Fullscreen) != 0)
        {
            sdlWindowFlags |= SDL_WINDOW_FULLSCREEN;
            _isFullscreen = true;
        }

        if ((flags & WindowFlags.FullscreenDesktop) != 0)
        {
            sdlWindowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            _isFullscreen = true;
        }

        if ((flags & WindowFlags.Maximized) != 0)
        {
            sdlWindowFlags |= SDL_WINDOW_MAXIMIZED;
        }

        _title = "Alimer";
#if SDL3
        SDLWindowHandle = SDL_CreateWindow(_title, 1200, 800, sdlWindowFlags);
        SDL_SetWindowPosition(SDLWindowHandle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#else
        SDLWindowHandle = SDL_CreateWindow(_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, sdlWindowFlags);
#endif
        Id = SDL_GetWindowID(SDLWindowHandle);
        SDL_GetWindowSizeInPixels(SDLWindowHandle, out int width, out int height);
        _clientSize = new Size(width, height);

        // Native handle
        SDL_SysWMinfo wmInfo = default;
#if SDL3
        SDL_GetWindowWMInfo(SDLWindowHandle, &wmInfo);
#else
        SDL_GetVersion(out wmInfo.version);
        SDL_GetWindowWMInfo(SDLWindowHandle, ref wmInfo);
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
    public override bool IsFullscreen
    {
        get => _isFullscreen;
        set
        {
            if (_isFullscreen != value)
            {
                _isFullscreen = value;
                if (_isFullscreen)
                    SDL_SetWindowFullscreen(SDLWindowHandle, (uint)SDL_WINDOW_FULLSCREEN_DESKTOP);
                else
                    SDL_SetWindowFullscreen(SDLWindowHandle, 0);
            }
        }
    }

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
        SDL_ShowWindow(SDLWindowHandle);
    }

    protected override void SetTitle(string title)
    {
        SDL_SetWindowTitle(SDLWindowHandle, title);
    }

    public void HandleEvent(in SDL_Event evt)
    {
        switch (evt.window.windowEvent)
        {
            case SDL_WINDOWEVENT_MINIMIZED: // SDL_EVENT_WINDOW_MINIMIZED
                _minimized = true;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_WINDOWEVENT_MAXIMIZED: // SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED: // SDL_EVENT_WINDOW_RESTORED:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_WINDOWEVENT_RESIZED: // SDL_EVENT_WINDOW_RESIZED:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_WINDOWEVENT_SIZE_CHANGED: // SDL_EVENT_WINDOW_CHANGED:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_WINDOWEVENT_CLOSE: // SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                Destroy();
                _platform.WindowClosed(evt.window.windowID);
                break;
        }
    }

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    private static extern nint GetModuleHandleW(ushort* lpModuleName);
}
