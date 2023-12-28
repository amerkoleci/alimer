// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Mathematics;
using Alimer.Graphics;
using static SDL.SDL_bool;
using System.Runtime.InteropServices;
using static SDL.SDL;
using SDL;

namespace Alimer;

internal unsafe class SDLWindow : Window
{
    private readonly SDLPlatform _platform;
    private SizeI _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly SDL_Window SDLWindowHandle;
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
        SDLWindowHandle = SDL_CreateWindow(_title, 1200, 800, sdlWindowFlags);
        if (SDLWindowHandle.IsNull)
        {
            Log.Error($"SDL_CreateWindow Failed: {SDL_GetErrorString()}");
            return;
        }

        Id = SDL_GetWindowID(SDLWindowHandle);
        SDL_SetWindowPosition(SDLWindowHandle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_GetWindowSizeInPixels(SDLWindowHandle, out int width, out int height);
        _clientSize = new(width, height);

        // Native handle
        if (OperatingSystem.IsWindows())
        {
            Kind = SwapChainSurfaceType.Win32;
            ContextHandle = GetModuleHandleW(null);
            Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.win32.hwnd");
        }
        else if (OperatingSystem.IsAndroid())
        {
            Kind = SwapChainSurfaceType.Android;
            ContextHandle = 0;
            Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.android.window");
        }
        else if (OperatingSystem.IsIOS())
        {
            Kind = SwapChainSurfaceType.MetalLayer;
            ContextHandle = 0;
            // the (__unsafe_unretained) UIWindow associated with the window
            Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.uikit.window");
        }
        else if (OperatingSystem.IsMacOS())
        {
            Kind = SwapChainSurfaceType.MetalLayer;
            ContextHandle = 0;
            // the (__unsafe_unretained) NSWindow associated with the window
            Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.cocoa.window");
        }
        else if (OperatingSystem.IsLinux())
        {
            Kind = SwapChainSurfaceType.Xlib;
            ContextHandle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.x11.display");
            Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.x11.window");
            // TODO: Wayland
            //ContextHandle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.wayland.display");
            //Handle = SDL_GetProperty(SDL_GetWindowProperties(SDLWindowHandle), "SDL.window.wayland.surface");
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
                SDL_SetWindowFullscreen(SDLWindowHandle, value ? SDL_TRUE : SDL_FALSE);
            }
        }
    }

    /// <inheritdoc />
    public override Int2 Position
    {
        get
        {
            SDL_GetWindowPosition(SDLWindowHandle, out int x, out int y);
            return new Int2(x, y);
        }
        set
        {
            SDL_SetWindowPosition(SDLWindowHandle, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public override SizeI ClientSize => _clientSize;

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
            case SDL_EventType.WindowMinimized:
                _minimized = true;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_EventType.WindowMaximized:
            case SDL_EventType.WindowRestored:
                _minimized = false;
                _clientSize = new(evt.window.data1, evt.window.data2);
                OnSizeChanged();
                break;

            case SDL_EventType.WindowResized:
                _minimized = false;
                HandleResize(evt);
                break;

            case SDL_EventType.WindowPixelSizeChanged:
                _minimized = false;
                HandleResize(evt);
                break;

            case SDL_EventType.WindowCloseRequested:
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
