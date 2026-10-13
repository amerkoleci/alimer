// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Numerics;
using Alimer.Utilities;
using static SDL3;
using static SDL3.SDL_SystemCursor;

namespace Alimer.Input;

internal unsafe class CursorSDL : Cursor, IDisposable
{
    public CursorSDL(SDL_Cursor* handle)
    {
        Handle = handle;
    }

    public CursorSDL(CursorType cursorType)
        : base(cursorType)
    {
        Handle = SDL_CreateSystemCursor(ToSDL(cursorType));
    }

    public CursorSDL(ReadOnlySpan<ColorRgba> pixels, in SizeI size, in PointI hotspot)
       : base(CursorType.Custom)
    {
        Debug.Assert(sizeof(ColorRgba) == 4);
        int pitch = size.Width * 4;
        SDL_Surface* surface = SDL_CreateSurfaceFrom(size.Width, size.Height, SDL_PixelFormat.SDL_PIXELFORMAT_RGBA8888, pixels.GetPointerUnsafe(), pitch);
        if (surface is null)
        {
            throw new InvalidOperationException();
        }

        Handle = SDL_CreateColorCursor(surface, hotspot.X, hotspot.Y);
        SDL_DestroySurface(surface);
    }

    public SDL_Cursor* Handle { get; private set; }

    ~CursorSDL() => Dispose();

    public void Dispose()
    {
        if (Handle is not null)
        {
            SDL_DestroyCursor(Handle);
            Handle = default;
        }

        GC.SuppressFinalize(this);
    }

    private static SDL_SystemCursor ToSDL(CursorType value)
    {
        return value switch
        {
            CursorType.Arrow => SDL_SYSTEM_CURSOR_DEFAULT,
            CursorType.Cross => SDL_SYSTEM_CURSOR_CROSSHAIR,
            CursorType.Hand => SDL_SYSTEM_CURSOR_POINTER,
            CursorType.Wait => SDL_SYSTEM_CURSOR_WAIT,
            CursorType.IBeam => SDL_SYSTEM_CURSOR_TEXT,
            CursorType.Progress => SDL_SYSTEM_CURSOR_PROGRESS,
            CursorType.ResizeNWSE => SDL_SYSTEM_CURSOR_NWSE_RESIZE,
            CursorType.ResizeNESW => SDL_SYSTEM_CURSOR_NESW_RESIZE,
            CursorType.ResizeEW => SDL_SYSTEM_CURSOR_EW_RESIZE,
            CursorType.ResizeNS => SDL_SYSTEM_CURSOR_NS_RESIZE,
            CursorType.Move => SDL_SYSTEM_CURSOR_MOVE,
            CursorType.NotAllowed => SDL_SYSTEM_CURSOR_NOT_ALLOWED,
            CursorType.Pointer => SDL_SYSTEM_CURSOR_POINTER,
            CursorType.ResizeNW => SDL_SYSTEM_CURSOR_NW_RESIZE,
            CursorType.ResizeN => SDL_SYSTEM_CURSOR_N_RESIZE,
            CursorType.ResizeNE => SDL_SYSTEM_CURSOR_NE_RESIZE,
            CursorType.ResizeE => SDL_SYSTEM_CURSOR_E_RESIZE,
            CursorType.ResizeSE => SDL_SYSTEM_CURSOR_SE_RESIZE,
            CursorType.ResizeS => SDL_SYSTEM_CURSOR_S_RESIZE,
            CursorType.ResizeSW => SDL_SYSTEM_CURSOR_SW_RESIZE,
            CursorType.ResizeW => SDL_SYSTEM_CURSOR_W_RESIZE,
            CursorType.Custom => throw new NotSupportedException("Custom cursor type is not supported."),
            _ => throw new ArgumentOutOfRangeException(nameof(value), value, null)
        };
    }
}
