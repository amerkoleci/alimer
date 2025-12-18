// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Platform/SDL/Window.SDL.h"
#include <SDL3/SDL.h>

namespace
{
    static uint32_t s_WindowCount = 0;
}

using namespace Alimer;

WindowSDL::WindowSDL(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags)
    : _fullscreen(false)
{
    SDL_WindowFlags sdl_flags = 0;
    if (CheckBitsAny(flags, WindowFlags::Fullscreen))
    {
        sdl_flags |= SDL_WINDOW_FULLSCREEN;
        _fullscreen = true;
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

    _id = SDL_GetWindowID(_handle);
    s_WindowCount++;
}

WindowSDL::~WindowSDL()
{
    if (_handle != nullptr)
    {
        SDL_DestroyWindow(_handle);
        _handle = nullptr;
    }

    s_WindowCount--;
}

void WindowSDL::SetTitle(std::string_view title)
{
    _title = title;
    SDL_SetWindowTitle(_handle, _title.c_str());
}

void WindowSDL::Show()
{
    //CreateSwapChain();
    SDL_ShowWindow(_handle);
}

void WindowSDL::Hide()
{
    SDL_HideWindow(_handle);
}

void WindowSDL::Maximize()
{
    SDL_MaximizeWindow(_handle);
}

void WindowSDL::Minimize()
{
    SDL_MinimizeWindow(_handle);
}

void WindowSDL::Restore()
{
    SDL_RestoreWindow(_handle);
}

bool WindowSDL::IsOpen() const
{
    return _handle != nullptr;
}

bool WindowSDL::IsMinimized() const
{
    return (SDL_GetWindowFlags(_handle) & SDL_WINDOW_MINIMIZED) != 0;
}

bool WindowSDL::IsFullscreen() const
{
    return (SDL_GetWindowFlags(_handle) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool WindowSDL::IsFocused() const
{
    return (SDL_GetWindowFlags(_handle) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool WindowSDL::IsCursorVisible() const
{
    return SDL_CursorVisible();
}

void WindowSDL::RequestFocus()
{
    SDL_RaiseWindow(_handle);
}
void WindowSDL::SetFullscreen(bool value)
{
    if (_fullscreen == value)
        return;

    if (!SDL_SetWindowFullscreen(_handle, value))
    {
        const char* result = SDL_GetError();
        SDL_ClearError();
        ::SDL_Log("%s", result);
    }

    _fullscreen = value;
}

void WindowSDL::SetCursorVisible(bool value)
{
    if (value)
    {
        SDL_ShowCursor();
    }
    else
    {
        SDL_HideCursor();
    }
}

void WindowSDL::OnResized()
{
    OnClientSizeChanged();
}
