// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Alimer.Utilities;
using Alimer.Platforms.Apple;
using static SDL3.SDL_WindowFlags;
using static SDL3.SDL_EventType;
using static SDL3;
using System.Diagnostics;

namespace Alimer;

internal unsafe sealed class WindowSDL : Window
{
    private string _title;
    private readonly SurfaceSource _surfaceSource;

    internal WindowSDL(RuntimePlatformSDL platform, int width, int height, WindowFlags flags)
    {
        Platform = platform;
        _title = "Alimer";

        bool fullscreen = flags.HasFlag(WindowFlags.Fullscreen) || flags.HasFlag(WindowFlags.ExclusiveFullscreen);

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

        Handle = SDL_CreateWindow(_title, width, height, windowFlags);
        if (Handle is null)
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

        Id = SDL_GetWindowID(Handle);
        if (!fullscreen)
        {
            SDL_SetWindowPosition(Handle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
        else if ((flags & WindowFlags.ExclusiveFullscreen) != 0)
        {
            SDL_DisplayMode mode = default;
            if (SDL_GetClosestFullscreenDisplayMode(SDL_GetPrimaryDisplay(), width, height, 0.0f, true, &mode))
            {
                SDL_SetWindowFullscreenMode(Handle, &mode).LogErrorIfFailed();
            }
            else
            {
                Log.Warn($"SDL: no fullscreen mode near {width}x{height}; the desktop's is used");
            }
        }

        //SDL_GetWindowSizeInPixels(Handle, out width, out height).LogErrorIfFailed();

        // https://github.com/eliemichel/sdl3webgpu/blob/main/sdl3webgpu.c
        // https://github.com/eliemichel/glfw3webgpu/blob/main/glfw3webgpu.c

        // Native handle
        SDL_PropertiesID props = SDL_GetWindowProperties(Handle);
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

    public RuntimePlatformSDL Platform { get; }
    public SDL_Window* Handle { get; private set; }
    public SDL_WindowID Id { get; }

    /// <inheritdoc />
    public override SurfaceSource SurfaceSource => _surfaceSource;

    /// <inheritdoc />
    public override string Title
    {
        get => _title;
        set
        {
            ArgumentException.ThrowIfNullOrEmpty(value, nameof(value));

            if (_title != value)
            {
                _title = value;
                SDL_SetWindowTitle(Handle, value);
            }
        }
    }

    /// <inheritdoc />
    public override WindowState State
    {
        get
        {
            SDL_WindowFlags flags = SDL_GetWindowFlags(Handle);
            if ((flags & SDL_WINDOW_FULLSCREEN) != 0)
            {
                if (SDL_GetWindowFullscreenMode(Handle) is not null)
                {
                    return WindowState.ExclusiveFullScreen;
                }
                else
                {
                    return WindowState.FullScreen;
                }
            }

            if ((flags & SDL_WINDOW_MINIMIZED) != 0)
            {
                return WindowState.Minimized;
            }
            if ((flags & SDL_WINDOW_MAXIMIZED) != 0)
            {
                return WindowState.Maximized;
            }

            return WindowState.Normal;
        }
        set
        {
            SDL_WindowFlags flags = SDL_GetWindowFlags(Handle);
            bool isFullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;

            // If the window is currently in fullscreen mode and the new state is not fullscreen, we need to exit fullscreen mode first.
            if (isFullscreen && value != WindowState.FullScreen && value != WindowState.ExclusiveFullScreen)
            {
                SDL_SetWindowFullscreenMode(Handle, null).LogErrorIfFailed();
                SDL_SetWindowFullscreen(Handle, false).LogErrorIfFailed();
            }

            switch (value)
            {
                case WindowState.Normal:
                    SDL_RestoreWindow(Handle).LogErrorIfFailed();
                    break;
                case WindowState.Minimized:
                    SDL_MinimizeWindow(Handle).LogErrorIfFailed();
                    break;
                case WindowState.Maximized:
                    SDL_MaximizeWindow(Handle).LogErrorIfFailed();
                    break;
                case WindowState.FullScreen:
                    SDL_SetWindowFullscreenMode(Handle, null).LogErrorIfFailed();
                    SDL_SetWindowFullscreen(Handle, true).LogErrorIfFailed();
                    break;
                case WindowState.ExclusiveFullScreen:
                    SDL_DisplayMode mode = default;
                    if (SDL_GetClosestFullscreenDisplayMode(SDL_GetPrimaryDisplay(), Size.Width, Size.Height, 0.0f, true, &mode))
                    {
                        SDL_SetWindowFullscreenMode(Handle, &mode);
                    }
                    else
                    {
                        SDL_DisplayID displayID = SDL_GetDisplayForWindow(Handle);
                        if (displayID > 0)
                        {
                            SDL_SetWindowFullscreenMode(Handle, SDL_GetCurrentDisplayMode(displayID)).LogErrorIfFailed();
                        }

                        SDL_SetWindowFullscreen(Handle, true).LogErrorIfFailed();
                        break;
                    }
                    break;
            }
        }
    }

    /// <inheritdoc />
    public override PointI Position
    {
        get
        {
            SDL_GetWindowPosition(Handle, out int x, out int y);
            return new(x, y);
        }
        set
        {
            SDL_SetWindowPosition(Handle, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public override SizeI Size
    {
        get
        {
            SDL_GetWindowSize(Handle, out int width, out int height);
            return new(width, height);
        }
        set
        {
            SDL_SetWindowSize(Handle, value.Width, value.Height);
        }
    }

    /// <inheritdoc />
    public override SizeI SizeInPixels
    {
        get
        {
            SDL_GetWindowSizeInPixels(Handle, out int width, out int height);
            return new(width, height);
        }
    }

    public override float ContentScale
    {
        get
        {
            // emscripten_get_device_pixel_ratio
            float scale = SDL_GetWindowDisplayScale(Handle);
            return scale > 0.0f ? scale : 1.0f; // SDL returns 0 before the window is shown
        }
    }

    internal void Destroy()
    {
        Surface?.Dispose();

        if (Handle is not null)
        {
            SDL_DestroyWindow(Handle);
            Handle = default;
        }
    }

    public void Show()
    {
        SDL_ShowWindow(Handle);
    }

    public void Hide()
    {
        SDL_HideWindow(Handle);
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
                Platform.WindowClosed(evt.windowID);
                break;
        }
    }

    private void HandleResize(in SDL_WindowEvent evt)
    {
        OnSizeChanged();
    }
}
