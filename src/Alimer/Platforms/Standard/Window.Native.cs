// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;
using Alimer.Utilities;
using CommunityToolkit.Diagnostics;
using Alimer.Platforms.Apple;

namespace Alimer;

partial class Window
{
    private readonly NativePlatform _platform;
    private readonly SwapChainSurface _surface;
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

        // https://github.com/eliemichel/sdl3webgpu/blob/main/sdl3webgpu.c
        // https://github.com/eliemichel/glfw3webgpu/blob/main/glfw3webgpu.c

        // Native handle
        if (OperatingSystem.IsWindows())
        {
            _surface = SwapChainSurface.CreateWin32(alimerGetWin32Window(_window));
        }
        else if (OperatingSystem.IsAndroid())
        {
            // TODO
            //Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindowHandle), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, IntPtr.Zero);
            throw new PlatformNotSupportedException();
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            NSWindow nswindow = alimerGetCocoaWindow(_window);

            NSView contentView = nswindow.contentView;

            if (!CAMetalLayer.TryCast(contentView.layer, out CAMetalLayer metalLayer))
            {
                metalLayer = CAMetalLayer.New();
                contentView.wantsLayer = true;
                contentView.layer = metalLayer;
            }

            _surface = SwapChainSurface.CreateMetalLayer(metalLayer.Handle);
        }
        else if (OperatingSystem.IsLinux())
        {
            nint wayland_display = alimerGetWaylandDisplay(_window);
            nint wayland_surface = alimerGetWaylandSurface(_window);

            if (wayland_display != 0 && wayland_surface != 0)
            {
                _surface = SwapChainSurface.CreateWayland(wayland_display, wayland_surface);
            }
            else
            {
                // X11
                nint x11_display = alimerGetX11Display(_window);
                ulong x11_window = alimerGetX11Window(_window);
                Guard.IsTrue(x11_display != 0 && x11_window != 0, "Failed to get X11 window information.");

                _surface = SwapChainSurface.CreateXlib(x11_display, x11_window);
            }
        }
        else
        {
            throw new PlatformNotSupportedException();
        }
    }

    /// <inheritdoc />
    public partial SwapChainSurface Surface => _surface;

    /// <inheritdoc />
    public partial bool IsMinimized
    {
        get => alimerWindowIsMinimized(_window);
    }

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
    public partial SizeI Size
    {
        get
        {
            alimerWindowGetSize(_window, out int width, out int height);
            return new(width, height);
        }
        set
        {
            alimerWindowSetSize(_window, value.Width, value.Height);
        }
    }

    /// <inheritdoc />
    public partial SizeI SizeInPixels
    {
        get
        {
            alimerWindowGetSizeInPixels(_window, out int width, out int height);
            return new(width, height);
        }
    }

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
                break;

            case WindowEventType.Maximized:
            case WindowEventType.Restored:
                break;

            case WindowEventType.Resized:
                HandleResize(evt);
                break;

            case WindowEventType.SizeChanged:
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
        OnSizeChanged();
    }
}
