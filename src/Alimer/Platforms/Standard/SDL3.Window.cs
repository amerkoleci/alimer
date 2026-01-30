// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using Alimer.Input;

namespace Alimer;

internal unsafe static partial class SDL3
{
    public const int SDL_WINDOWPOS_CENTERED = (0x2FFF0000 | (0));

    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER => "SDL.window.android.window"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WIN32_HWND_POINTER => "SDL.window.win32.hwnd"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WIN32_HDC_POINTER => "SDL.window.win32.hdc"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER => "SDL.window.win32.instance"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER => "SDL.window.wayland.display"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER => "SDL.window.wayland.surface"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_VIEWPORT_POINTER => "SDL.window.wayland.viewport"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER => "SDL.window.wayland.egl_window"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_XDG_SURFACE_POINTER => "SDL.window.wayland.xdg_surface"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_XDG_TOPLEVEL_POINTER => "SDL.window.wayland.xdg_toplevel"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_XDG_TOPLEVEL_EXPORT_HANDLE_STRING => "SDL.window.wayland.xdg_toplevel_export_handle"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_XDG_POPUP_POINTER => "SDL.window.wayland.xdg_popup"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_WAYLAND_XDG_POSITIONER_POINTER => "SDL.window.wayland.xdg_positioner"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_X11_DISPLAY_POINTER => "SDL.window.x11.display"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_X11_SCREEN_NUMBER => "SDL.window.x11.screen"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_X11_WINDOW_NUMBER => "SDL.window.x11.window"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_EMSCRIPTEN_CANVAS_ID_STRING => "SDL.window.emscripten.canvas_id"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_EMSCRIPTEN_FILL_DOCUMENT_BOOLEAN => "SDL.window.emscripten.fill_document"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_EMSCRIPTEN_KEYBOARD_ELEMENT_STRING => "SDL.window.emscripten.keyboard_element"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_COCOA_WINDOW_POINTER => "SDL.window.cocoa.window"u8;
    public static ReadOnlySpan<byte> SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER => "SDL.window.uikit.window"u8;

    [Flags]
    public enum SDL_WindowFlags : ulong
    {
        SDL_WINDOW_FULLSCREEN = 0x1,
        SDL_WINDOW_OPENGL = 0x2,
        SDL_WINDOW_OCCLUDED = 0x4,
        SDL_WINDOW_HIDDEN = 0x08,
        SDL_WINDOW_BORDERLESS = 0x10,
        SDL_WINDOW_RESIZABLE = 0x20,
        SDL_WINDOW_MINIMIZED = 0x40,
        SDL_WINDOW_MAXIMIZED = 0x080,
        SDL_WINDOW_MOUSE_GRABBED = 0x100,
        SDL_WINDOW_INPUT_FOCUS = 0x200,
        SDL_WINDOW_MOUSE_FOCUS = 0x400,
        SDL_WINDOW_EXTERNAL = 0x0800,
        SDL_WINDOW_MODAL = 0x1000,
        SDL_WINDOW_HIGH_PIXEL_DENSITY = 0x2000,
        SDL_WINDOW_MOUSE_CAPTURE = 0x4000,
        SDL_WINDOW_MOUSE_RELATIVE_MODE = 0x08000,
        SDL_WINDOW_ALWAYS_ON_TOP = 0x10000,
        SDL_WINDOW_UTILITY = 0x20000,
        SDL_WINDOW_TOOLTIP = 0x40000,
        SDL_WINDOW_POPUP_MENU = 0x080000,
        SDL_WINDOW_KEYBOARD_GRABBED = 0x100000,
        SDL_WINDOW_VULKAN = 0x10000000,
        SDL_WINDOW_METAL = 0x20000000,
        SDL_WINDOW_TRANSPARENT = 0x40000000,
        SDL_WINDOW_NOT_FOCUSABLE = 0x080000000,
    }

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_CreateWindow(string title, int w, int h, SDL_WindowFlags flags);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial uint SDL_GetWindowID(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial nint SDL_GetWindowFromID(uint id);


    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetWindowIcon(nint window, nint icon);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetWindowPosition(nint window, int x, int y);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetWindowPosition(nint window, out int x, out int y);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetWindowSize(nint window, int w, int h);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetWindowSize(nint window, out int w, out int h);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_GetWindowSizeInPixels(nint window, out int w, out int h);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_WindowFlags SDL_GetWindowFlags(nint window);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetWindowTitle(nint window, string title);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_ShowWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_HideWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_RaiseWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_MaximizeWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_MinimizeWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_RestoreWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SetWindowFullscreen(nint window, SDLBool fullscreen);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDLBool SDL_SyncWindow(nint window);

    [LibraryImport(LibraryName)]
    [UnmanagedCallConv(CallConvs = [typeof(CallConvCdecl)])]
    public static partial SDL_PropertiesID SDL_GetWindowProperties(nint window);

}
