// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Window.h"

struct SDL_Window;

namespace Alimer
{
    class WindowImpl final
    {
    public:
        WindowImpl(const std::string& title, uint32_t width, uint32_t height, WindowFlags flags);
        ~WindowImpl();

        // Non-copyable and non-movable
        ALIMER_DISABLE_COPY_MOVE(WindowImpl);

        SDL_Window* GetHandle() const noexcept { return _handle; }

    private:
        SDL_Window* _handle;
    };
}
