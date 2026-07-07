// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using System.Runtime.InteropServices;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_WindowFlags;
using static Alimer.SDL3.SDL_EventType;
using Alimer.Utilities;
using Alimer.Platforms.Apple;
using System.Diagnostics;

namespace Alimer;

unsafe partial class Window
{
    private readonly SDLPlatform _platform;
    private readonly SurfaceSource _surfaceSource;
    private bool _isFullscreen;
    private SDL_Window* _handle;

    internal Window(SDLPlatform platform, WindowFlags flags)
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

        _handle = SDL_CreateWindow(_title, 1280, 720, windowFlags);
        if (_handle == null)
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

        // https://github.com/eliemichel/sdl3webgpu/blob/main/sdl3webgpu.c
        // https://github.com/eliemichel/glfw3webgpu/blob/main/glfw3webgpu.c

        // Native handle
        SDL_PropertiesID props = SDL_GetWindowProperties(_handle);
        if (OperatingSystem.IsWindows())
        {
            nint hwnd = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER);
            _surfaceSource = SurfaceSource.CreateWin32(hwnd);
        }
        else if (OperatingSystem.IsAndroid())
        {
            nint androidWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER);
            _surfaceSource = SurfaceSource.CreateAndroid(androidWindow);
        }
        else if (OperatingSystem.IsIOS())
        {
            UIWindow uiWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER);
            UIView uiView = uiWindow.RootViewController.View;

            if (!CAMetalLayer.TryCast(uiView.layer, out CAMetalLayer metalLayer))
            {
                metalLayer = CAMetalLayer.New();
                metalLayer.opaque = true;
                metalLayer.frame = uiView.frame;
                metalLayer.drawableSize = uiView.frame.size;

                uiView.layer.addSublayer(metalLayer.Handle);
            }

            _surfaceSource = SurfaceSource.CreateMetalLayer(metalLayer.Handle);
        }
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            NSWindow nsWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER);

            NSView contentView = nsWindow.contentView;

            if (!CAMetalLayer.TryCast(contentView.layer, out CAMetalLayer metalLayer))
            {
                metalLayer = CAMetalLayer.New();
                contentView.wantsLayer = true;
                contentView.layer = metalLayer;
            }

            _surfaceSource = SurfaceSource.CreateMetalLayer(metalLayer.Handle);
        }
        else if (OperatingSystem.IsLinux())
        {
            if (SDL_GetCurrentVideoDriver().Equals("x11", StringComparison.OrdinalIgnoreCase))
            {
                // X11
                nint x11Display = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER);
                ulong x11Window = (ulong)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER);
                Debug.Assert(x11Display != 0 && x11Window != 0, "Failed to get X11 window information.");

                _surfaceSource = SurfaceSource.CreateXlib(x11Display, x11Window);
            }
            else if (SDL_GetCurrentVideoDriver().Equals("wayland", StringComparison.OrdinalIgnoreCase))
            {
                nint waylandDisplay = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER);
                nint waylandSurface = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER);

                _surfaceSource = SurfaceSource.CreateWayland(waylandDisplay, waylandSurface);
            }
            else
            {
                throw new PlatformNotSupportedException();
            }
        }
        else
        {
            throw new PlatformNotSupportedException();
        }
    }

    internal SDL_WindowID Id { get; }

    /// <inheritdoc />
    public partial SurfaceSource SurfaceSource => _surfaceSource;

    /// <inheritdoc />
    public partial bool IsMinimized
    {
        get
        {
            if (_handle == null)
                return true;

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
            SDL_GetWindowPosition(_handle, out int x, out int y);
            return new(x, y);
        }
        set
        {
            SDL_SetWindowPosition(_handle, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public partial SizeI Size
    {
        get
        {
            SDL_GetWindowSize(_handle, out int width, out int height);
            return new(width, height);
        }
        set
        {
            SDL_SetWindowSize(_handle, value.Width, value.Height);
        }
    }

    /// <inheritdoc />
    public partial SizeI SizeInPixels
    {
        get
        {
            SDL_GetWindowSizeInPixels(_handle, out int width, out int height);
            return new(width, height);
        }
    }

    internal void Destroy()
    {
        Surface?.Dispose();

        if (_handle != null)
        {
            SDL_DestroyWindow(_handle);
            _handle = null;
        }
    }

    public void Show()
    {
        SDL_ShowWindow(_handle);
    }

    public void Hide()
    {
        SDL_HideWindow(_handle);
    }

    public void Minimize()
    {
        SDL_MinimizeWindow(_handle);
    }

    public void Maximize()
    {
        SDL_MaximizeWindow(_handle);
    }

    public void Restore()
    {
        SDL_RestoreWindow(_handle);
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
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                HandleResize(evt);
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
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
        OnSizeChanged();
    }
}
