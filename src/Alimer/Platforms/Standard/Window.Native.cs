// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using static Alimer.AlimerApi;
using Alimer.Utilities;
using Alimer.Platforms.Apple;

namespace Alimer;

unsafe partial class Window
{
    private readonly SDLPlatform _platform;
    private readonly SurfaceSource _surfaceSource;
    private bool _isFullscreen;
    private nint _handle;

    internal Window(SDLPlatform platform, WindowFlags flags)
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

        _handle = alimerWindowCreate(in desc);
        if (_handle == 0)
        {
            throw new InvalidOperationException($"Alimer: alimerWindowCreate failed");
        }

        _isFullscreen = flags.HasFlag(WindowFlags.Fullscreen);
        Id = alimerWindowGetID(_handle);
        alimerWindowSetCentered(_handle);
        alimerWindowGetSize(_handle, out int width, out int height);

        // https://github.com/eliemichel/sdl3webgpu/blob/main/sdl3webgpu.c
        // https://github.com/eliemichel/glfw3webgpu/blob/main/glfw3webgpu.c

        // Native handle
        if (OperatingSystem.IsWindows())
        {
            nint hwnd = alimerWindowGetNativeHandle(_handle);
            _surfaceSource = SurfaceSource.CreateWin32(hwnd);
        }
        else if (OperatingSystem.IsAndroid())
        {
            nint androidWindow = alimerWindowGetNativeHandle(_handle);
            _surfaceSource = SurfaceSource.CreateAndroid(androidWindow);
        }
#if TODO
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
#endif
        else if (OperatingSystem.IsMacOS() || OperatingSystem.IsMacCatalyst())
        {
            NSWindow nsWindow = alimerWindowGetNativeHandle(_handle);

            NSView contentView = nsWindow.contentView;

            if (!CAMetalLayer.TryCast(contentView.layer, out CAMetalLayer metalLayer))
            {
                metalLayer = CAMetalLayer.New();
                contentView.wantsLayer = true;
                contentView.layer = metalLayer;
            }

            _surfaceSource = SurfaceSource.CreateMetalLayer(metalLayer.Handle);
        }
#if TODO
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
#endif // We need to handle on native side
        else
        {
            throw new PlatformNotSupportedException();
        }
    }

    internal uint Id { get; }

    /// <inheritdoc />
    public partial SurfaceSource SurfaceSource => _surfaceSource;

    /// <inheritdoc />
    public partial bool IsMinimized
    {
        get
        {
            if (_handle == null)
                return true;

            return alimerWindowIsMinimized(_handle);
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
                alimerWindowSetFullscreen(_handle, value);
            }
        }
    }

    /// <inheritdoc />
    public partial PointI Position
    {
        get
        {
            alimerWindowGetPosition(_handle, out int x, out int y);
            return new(x, y);
        }
        set
        {
            alimerWindowSetPosition(_handle, value.X, value.Y);
        }
    }

    /// <inheritdoc />
    public partial SizeI Size
    {
        get
        {
            alimerWindowGetSize(_handle, out int width, out int height);
            return new(width, height);
        }
        set
        {
            alimerWindowSetSize(_handle, value.Width, value.Height);
        }
    }

    /// <inheritdoc />
    public partial SizeI SizeInPixels
    {
        get
        {
            alimerWindowGetSizeInPixels(_handle, out int width, out int height);
            return new(width, height);
        }
    }

    internal void Destroy()
    {
        Surface?.Dispose();

        if (_handle != 0)
        {
            alimerWindowDestroy(_handle);
            _handle = 0;
        }
    }

    public void Show()
    {
        alimerWindowShow(_handle);
    }

    public void Hide()
    {
        alimerWindowHide(_handle);
    }

    public void Minimize()
    {
        alimerWindowMinimize(_handle);
    }

    public void Maximize()
    {
        alimerWindowMaximize(_handle);
    }

    public void Restore()
    {
        alimerWindowRestore(_handle);
    }

    private partial void SetTitle(string title)
    {
        alimerWindowSetTitle(_handle, title);
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
