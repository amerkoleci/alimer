// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Platform/SDL/Window.SDL.h"
#include <SDL3/SDL.h>

using namespace Alimer;

WindowImpl::WindowImpl(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags)
{
    SDL_WindowFlags sdl_flags = 0;
    if (CheckBitsAny(flags, WindowFlags::Fullscreen))
    {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (CheckBitsAny(flags, WindowFlags::Hidden))
    {
        sdl_flags |= SDL_WINDOW_HIDDEN;
    }

    if (CheckBitsAny(flags, WindowFlags::Borderless))
    {
        sdl_flags |= SDL_WINDOW_BORDERLESS;
    }

    if (CheckBitsAny(flags, WindowFlags::Resizable))
    {
        sdl_flags |= SDL_WINDOW_RESIZABLE;
    }

    if (CheckBitsAny(flags, WindowFlags::Maximized))
    {
        sdl_flags |= SDL_WINDOW_MINIMIZED;
    }
    if (CheckBitsAny(flags, WindowFlags::AlwaysOnTop))
    {
        sdl_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    }
    if (CheckBitsAny(flags, WindowFlags::Transparent))
    {
        sdl_flags |= SDL_WINDOW_TRANSPARENT;
    }

    _handle = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), sdl_flags);
    if (_handle == nullptr)
    {
        LOGE("Failed to create SDL windows");
        return;
    }
}

WindowImpl::~WindowImpl()
{
    if (_handle != nullptr)
    {
        SDL_DestroyWindow(_handle);
        _handle = nullptr;
    }
}
