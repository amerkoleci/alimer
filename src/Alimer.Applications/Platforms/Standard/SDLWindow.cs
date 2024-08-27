// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Mathematics;
using Alimer.Graphics;
using System.Runtime.InteropServices;
using static SDL3.SDL3;
using SDL3;

namespace Alimer;

internal unsafe class SDLWindow : Window
{
    private readonly SDLPlatform _platform;
    private SizeI _clientSize;
    private bool _minimized;
    private bool _isFullscreen;

    public readonly SDL_Window SDLWindowHandle;
    public readonly SDL_WindowID Id;

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
            Log.Error($"SDL_CreateWindow Failed: {SDL_GetError()}");
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
            Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_WIN32_HWND_POINTER, IntPtr.Zero);
        }
        else if (OperatingSystem.IsAndroid())
        {
            Kind = SwapChainSurfaceType.Android;
            ContextHandle = 0;
            Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, IntPtr.Zero);
        }
        else if (OperatingSystem.IsIOS())
        {
            Kind = SwapChainSurfaceType.MetalLayer;
            ContextHandle = 0;
            // the (__unsafe_unretained) UIWindow associated with the window
            Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, IntPtr.Zero);
        }
        else if (OperatingSystem.IsMacOS())
        {
            Kind = SwapChainSurfaceType.MetalLayer;
            ContextHandle = 0;
            // the (__unsafe_unretained) NSWindow associated with the window
            Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, IntPtr.Zero);
        }
        else if (OperatingSystem.IsLinux())
        {
            if (SDL_GetCurrentVideoDriver() == "wayland")
            {
                // Wayland
                Kind = SwapChainSurfaceType.Wayland;
                ContextHandle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, IntPtr.Zero);
                Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, IntPtr.Zero);
            }
            else
            {
                // X11
                Kind = SwapChainSurfaceType.Xlib;
                ContextHandle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, IntPtr.Zero);
                Handle = new IntPtr(SDL_GetNumberProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
            }

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
    public override Int2 Position
    {
        get
        {
            _ = SDL_GetWindowPosition(SDLWindowHandle, out int x, out int y);
            return new Int2(x, y);
        }
        set
        {
            _ = SDL_SetWindowPosition(SDLWindowHandle, value.X, value.Y);
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
        _ = SDL_ShowWindow(SDLWindowHandle);
    }

    protected override void SetTitle(string title)
    {
        _ = SDL_SetWindowTitle(SDLWindowHandle, title);
    }

    public void HandleEvent(in SDL_WindowEvent evt)
    {
        switch (evt.type)
        {
            case SDL_EVENT_WINDOW_MINIMIZED:
                _minimized = true;
                _clientSize = new(evt.data1, evt.data2);
                OnSizeChanged();
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
                _minimized = false;
                _clientSize = new(evt.data1, evt.data2);
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
                _platform.WindowClosed(evt.windowID);
                break;
        }
    }

    private void HandleResize(in SDL_WindowEvent evt)
    {
        if (_clientSize.Width != evt.data1 ||
            _clientSize.Height != evt.data2)
        {
            _clientSize = new(evt.data1, evt.data2);
            OnSizeChanged();
        }
    }

    [DllImport("kernel32", ExactSpelling = true)]
    //[SetsLastSystemError]
    private static extern nint GetModuleHandleW(ushort* lpModuleName);
}
