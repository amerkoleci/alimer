// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Platform/SDL/Window.SDL.h"
#include <SDL3/SDL.h>
#if defined(SDL_PLATFORM_MACOS)
#include <SDL3/SDL_metal.h>
#endif

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

#if defined(ALIMER_RHI_METAL)
    sdl_flags |= SDL_WINDOW_METAL;
#elif defined(ALIMER_RHI_VULKAN)
    sdl_flags |= SDL_WINDOW_VULKAN;
#endif

    _impl->handle = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), sdl_flags);
    if (_impl->handle == nullptr)
    {
        LOGE("Failed to create SDL windows");
        return;
    }

#if defined(__APPLE__)
    _impl->view = SDL_Metal_CreateView(_impl->handle);
#endif

    _id = SDL_GetWindowID(_impl->handle);
    s_WindowCount++;
}

Window::~Window()
{
    DestroySwapChain();

    if (_impl->handle != nullptr)
    {
#if defined(__APPLE__)
        if (_impl->view)
        {
            SDL_Metal_DestroyView(_impl->view);
            _impl->view = nullptr;
        }
#endif

        SDL_DestroyWindow(_impl->handle);
        _impl->handle = nullptr;
    }

    delete _impl;

    s_WindowCount--;
}

UInt2 Window::GetSize() const
{
    int width, height;
    SDL_GetWindowSize(_impl->handle, &width, &height);

    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

UInt2 Window::GetSizeInPixels() const
{
    int width, height;
    SDL_GetWindowSizeInPixels(_impl->handle, &width, &height);

    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
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


void Window::CreateSurface()
{
    [[maybe_unused]] SDL_PropertiesID properties = SDL_GetWindowProperties(_impl->handle);

#if defined(SDL_PLATFORM_WIN32)
    void* hwnd = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    _surface = GRHIFactory->CreateSurface(hwnd, nullptr);
#elif defined(SDL_PLATFORM_MACOS)
    void* layer = SDL_Metal_GetLayer(_impl->view);
    _surface = GRHIFactory->CreateSurface(layer, nullptr);
#elif defined(SDL_PLATFORM_ANDROID)
    void* window = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr);
    _surface = GRHIFactory->CreateSurface(window, nullptr);
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0)
    {
        void* xdisplay = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
        Sint64 xwindow = SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow)
        {
        }
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0)
    {
        struct wl_display* display = (struct wl_display*)SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr);
        struct wl_surface* surface = (struct wl_surface*)SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr);
        if (display && surface)
        {
            _surface = GRHIFactory->CreateSurface(surface, display);
        }
    }
#endif
}
