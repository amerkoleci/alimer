// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using System.Runtime.InteropServices;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_WindowFlags;
using static Alimer.SDL3.SDL_EventType;
using Alimer.Utilities;
using CommunityToolkit.Diagnostics;
using Alimer.Platforms.Apple;

namespace Alimer;

partial class Window
{
    private readonly NativePlatform _platform;
    private readonly SwapChainSurface _surface;
    private SizeI _clientSize;
    private bool _isFullscreen;
    private readonly nint _handle;
    public readonly uint Id;

    internal unsafe Window(NativePlatform platform, WindowFlags flags)
    {
        _platform = platform;
        _title = "Alimer";

        bool fullscreen = flags.HasFlag(WindowFlags.Fullscreen);

        SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN;

        if (fullscreen)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN;
        }
        else
        {
            if (flags.HasFlag(WindowFlags.Hidden))
                windowFlags |= SDL_WINDOW_HIDDEN;

            if (flags.HasFlag(WindowFlags.Borderless))
                windowFlags |= SDL_WINDOW_BORDERLESS;

            if (flags.HasFlag(WindowFlags.Resizable))
                windowFlags |= SDL_WINDOW_RESIZABLE;

            if (flags.HasFlag(WindowFlags.Maximized))
                windowFlags |= SDL_WINDOW_MAXIMIZED;

            if (flags.HasFlag(WindowFlags.AlwaysOnTop))
                windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
        }

        _handle = SDL_CreateWindow(_title, 1200, 800, windowFlags);
        if (_handle == 0)
        {
            throw new InvalidOperationException($"Alimer: SDL_CreateWindow Failed: {SDL_GetError()}");
        }

#if TODO
        if (desc->icon.data)
        {
            SDL_Surface* surface = SDL_CreateSurfaceFrom(
                static_cast<int>(desc->icon.width),
                static_cast<int>(desc->icon.height),
                SDL_PIXELFORMAT_RGBA8888,
                (void*)desc->icon.data,
                static_cast<int>(desc->icon.width * 4));
            if (!surface)
            {
                alimerLogError(LogCategory_Platform, "Alimer: SDL_CreateSurfaceFrom Failed: %s", SDL_GetError());
                SDL_DestroyWindow(handle);
                return nullptr;
            }

            SDL_SetWindowIcon(handle, surface);
            SDL_DestroySurface(surface);
        } 
#endif

        Id = SDL_GetWindowID(_handle);
        SDL_SetWindowPosition(_handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        _ = SDL_GetWindowSizeInPixels(_handle, out int width, out int height);
        _clientSize = new(width, height);

        // https://github.com/eliemichel/sdl3webgpu/blob/main/sdl3webgpu.c
        // https://github.com/eliemichel/glfw3webgpu/blob/main/glfw3webgpu.c

        // Native handle
        var props = SDL_GetWindowProperties(_handle);
        if (OperatingSystem.IsWindows())
        {
            nint hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER);
            _surface = SwapChainSurface.CreateWin32(hwnd);
        }
        else if (OperatingSystem.IsAndroid())
        {
            nint androidWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER);
            _surface = SwapChainSurface.CreateAndroid(androidWindow);
        }
        else if (OperatingSystem.IsIOS())
        {
            nint ui_window = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER);
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            NSWindow nswindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER);

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
            if (SDL_GetCurrentVideoDriver().Equals("x11", StringComparison.OrdinalIgnoreCase))
            {
                nint x11_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER);
                long x11_window = SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER);
                Guard.IsTrue(x11_display != 0 && x11_window != 0, "Failed to get X11 window information.");

                _surface = SwapChainSurface.CreateXlib(x11_display, (ulong)x11_window);
            }
            else if (SDL_GetCurrentVideoDriver().Equals("wayland", StringComparison.OrdinalIgnoreCase))
            {
                nint wayland_display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER);
                nint wayland_surface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER);
                _surface = SwapChainSurface.CreateWayland(wayland_display, wayland_surface);
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
        get
        {
            SDL_WindowFlags flags = SDL_GetWindowFlags(_handle);
            return (flags & SDL_WINDOW_MINIMIZED) != 0;
        }
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
                _ = SDL_SetWindowFullscreen(_handle, value);
            }
        }
    }

    /// <inheritdoc />
    public partial PointI Position
    {
        get
        {
            _ = SDL_GetWindowPosition(_handle, out int x, out int y);
            return new(x, y);
        }
        set
        {
            _ = SDL_SetWindowPosition(_handle, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public partial SizeI ClientSize => _clientSize;

    public void Show()
    {
        SDL_ShowWindow(_handle);
    }


    public void Hide()
    {
        SDL_HideWindow(_handle);
    }

    public void Maximize()
    {

        SDL_MaximizeWindow(_handle);
    }

    public void Minimize()
    {
        SDL_MinimizeWindow(_handle);
    }

    public void Restore()
    {
        SDL_RestoreWindow(_handle);
    }

    public void Focus()
    {
        SDL_RaiseWindow(_handle);
    }

    public bool HasFocus()
    {
        SDL_WindowFlags flags = SDL_GetWindowFlags(_handle);
        return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
    }


    private partial void SetTitle(string title)
    {
        SDL_SetWindowTitle(_handle, title);
    }

    internal void HandleEvent(in SDL_WindowEvent evt)
    {
        switch (evt.type)
        {
            case SDL_EVENT_WINDOW_MINIMIZED:
                //_minimized = true;
                _clientSize = new(evt.data1, evt.data2);
                OnSizeChanged();
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
                //_minimized = false;
                _clientSize = new(evt.data1, evt.data2);
                OnSizeChanged();
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                //_minimized = false;
                HandleResize(evt);
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                //_minimized = false;
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
}
