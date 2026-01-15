// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;
using Alimer.Utilities;

namespace Alimer;

partial class Window
{
    private readonly NativePlatform _platform;
    private SizeI _clientSize;
    private bool _minimized;
    private bool _isFullscreen;
    private readonly nint _window;
    public readonly uint Id;

    internal unsafe Window(NativePlatform platform, WindowFlags flags)
    {
        _platform = platform;
        _title = "Alimer";

        WindowDesc desc = new()
        {
            title = Utf8CustomMarshaller.ConvertToUnmanaged(_title),
            width = 1200,
            height = 800,
            flags = flags
        };

        _window = alimerWindowCreate(in desc);
        if (_window == 0)
        {
            throw new InvalidOperationException("Failed to create window");
        }

        Id = alimerWindowGetID(_window);
        alimerWindowSetCentered(_window);
        alimerWindowGetSizeInPixels(_window, out int width, out int height);
        _clientSize = new(width, height);

        // Native handle
        if (OperatingSystem.IsWindows())
        {
            Kind = SwapChainSurfaceType.Win32;
            ContextHandle = GetModuleHandleW(null);
            Handle = alimerWindowGetNativeHandle(_window);
        }
#if TODO
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
#endif
    }

    /// <inheritdoc />
    public partial bool IsMinimized => _minimized;

    /// <inheritdoc />
    public partial bool IsFullscreen
    {
        get => _isFullscreen;
        set
        {
            if (_isFullscreen != value)
            {
                _isFullscreen = value;
                alimerWindowSetFullscreen(_window, value);
            }
        }
    }

    /// <inheritdoc />
    public partial PointI Position
    {
        get
        {
            alimerWindowGetPosition(_window, out int x, out int y);
            return new(x, y);
        }
        set
        {
            alimerWindowSetPosition(_window, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public partial SizeI ClientSize => _clientSize;

    public void Show()
    {
        alimerWindowShow(_window);
    }

    private partial void SetTitle(string title)
    {
        alimerWindowSetTitle(_window, title);
    }

    internal void HandleEvent(in WindowEvent evt)
    {
        switch (evt.type)
        {
            case WindowEventType.Minimized:
                _minimized = true;
                _clientSize = new(evt.data1, evt.data2);
                OnSizeChanged();
                break;

            case WindowEventType.Maximized:
            case WindowEventType.Restored:
                _minimized = false;
                _clientSize = new(evt.data1, evt.data2);
                OnSizeChanged();
                break;

            case WindowEventType.Resized:
                _minimized = false;
                HandleResize(evt);
                break;

            case WindowEventType.SizeChanged:
                _minimized = false;
                HandleResize(evt);
                break;

            case WindowEventType.CloseRequested:
                Destroy();
                _platform.WindowClosed(evt.windowID);
                break;
        }
    }

    private void HandleResize(in WindowEvent evt)
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
    private static extern unsafe nint GetModuleHandleW(ushort* lpModuleName);
}
