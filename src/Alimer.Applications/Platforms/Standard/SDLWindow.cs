// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Graphics;
using static SDL.SDL_EventType;
using System.Runtime.InteropServices;
using static SDL.SDL;
using SDL;

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

        SDL_WindowFlags sdlWindowFlags = SDL_WindowFlags.HighPixelDensity | SDL_WindowFlags.Hidden | SDL_WindowFlags.Vulkan;

        if ((flags & WindowFlags.Borderless) != 0)
            sdlWindowFlags |= SDL_WindowFlags.Borderless;

        if ((flags & WindowFlags.Resizable) != 0)
            sdlWindowFlags |= SDL_WindowFlags.Resizable;

        if ((flags & WindowFlags.Fullscreen) != 0)
        {
            sdlWindowFlags |= SDL_WindowFlags.Fullscreen;
            _isFullscreen = true;
        }

        if ((flags & WindowFlags.Maximized) != 0)
        {
            sdlWindowFlags |= SDL_WindowFlags.Maximized;
        }

        _title = "Alimer";
        SDLWindowHandle = SDL_CreateWindowWithPosition(_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, sdlWindowFlags);
        Id = SDL_GetWindowID(SDLWindowHandle);
        SDL_GetWindowSizeInPixels(SDLWindowHandle, out int width, out int height);
        _clientSize = new Size(width, height);

        // Native handle
        SDL_SysWMinfo wmInfo = default;
        SDL_GetWindowWMInfo(SDLWindowHandle, &wmInfo);

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
                SDL_SetWindowFullscreen(SDLWindowHandle, value);
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
                HandleResize(evt);
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                _minimized = false;
                HandleResize(evt);
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                Destroy();
                _platform.WindowClosed(evt.window.windowID);
                break;
        }
    }

    private void HandleResize(in SDL_Event evt)
    {
        if (_clientSize.Width != evt.window.data1 ||
            _clientSize.Height != evt.window.data2)
        {
            _clientSize = new(evt.window.data1, evt.window.data2);
            OnSizeChanged();
        }
    }

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    private static extern nint GetModuleHandleW(ushort* lpModuleName);
}
