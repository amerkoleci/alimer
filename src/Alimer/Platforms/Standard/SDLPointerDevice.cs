// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;

namespace Alimer.Input;

internal class SDLPointerDevice : PointerDevice
{
    private readonly SDLCursor _defaultCursor;
    private SDLCursor _currentCursor;

    public SDLPointerDevice()
    {
        _defaultCursor = new(SDL_GetDefaultCursor());
        _currentCursor = new(SDL_GetCursor());
    }

    public override Cursor Cursor
    {
        get => _currentCursor;
        set
        {
            if (_currentCursor == value)
                return;

            _currentCursor = (SDLCursor)value;
            SDL_SetCursor(_currentCursor.Handle);
        }
    }
}
