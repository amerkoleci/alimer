// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Window.h"
typedef struct SDL_Window SDL_Window;
typedef void* SDL_MetalView;

namespace Alimer
{
    struct WindowImpl final
    {
        SDL_Window* handle = nullptr;
        SDL_MetalView view = nullptr;
    };
}
