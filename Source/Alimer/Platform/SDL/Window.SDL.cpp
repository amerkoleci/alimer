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

Window::Window(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags)
    : _impl(new WindowImpl())
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

    _impl->handle = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), sdl_flags);
    if (_impl->handle == nullptr)
    {
        LOGE("Failed to create SDL windows");
        return;
    }

    _id = SDL_GetWindowID(_impl->handle);
    s_WindowCount++;
}

Window::~Window()
{
    if (_impl->handle != nullptr)
    {
        SDL_DestroyWindow(_impl->handle);
        _impl->handle = nullptr;
    }

    delete _impl;

    s_WindowCount--;
}

void Window::SetTitle(std::string_view title)
{
    _title = title;
    SDL_SetWindowTitle(_impl->handle, _title.c_str());
}

void Window::Show()
{
    //CreateSwapChain();
    SDL_ShowWindow(_impl->handle);
}

void Window::Hide()
{
    SDL_HideWindow(_impl->handle);
}

void Window::Maximize()
{
    SDL_MaximizeWindow(_impl->handle);
}

void Window::Minimize()
{
    SDL_MinimizeWindow(_impl->handle);
}

void Window::Restore()
{
    SDL_RestoreWindow(_impl->handle);
}

bool Window::IsOpen() const
{
    return _impl->handle != nullptr;
}

bool Window::IsMinimized() const
{
    return (SDL_GetWindowFlags(_impl->handle) & SDL_WINDOW_MINIMIZED) != 0;
}

bool Window::IsFullscreen() const
{
    return (SDL_GetWindowFlags(_impl->handle) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool Window::IsFocused() const
{
    return (SDL_GetWindowFlags(_impl->handle) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool Window::IsCursorVisible() const
{
    return SDL_CursorVisible();
}

void Window::RequestFocus()
{
    SDL_RaiseWindow(_impl->handle);
}

void Window::SetFullscreen(bool value)
{
    if (!SDL_SetWindowFullscreen(_impl->handle, value))
    {
        const char* result = SDL_GetError();
        SDL_ClearError();
        ::SDL_Log("%s", result);
    }
}

void Window::SetCursorVisible(bool value)
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
