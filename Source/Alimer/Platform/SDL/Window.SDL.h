// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Platform/Window.h"
struct SDL_Window;

namespace Alimer
{
    struct WindowImpl final
    {
        SDL_Window* handle = nullptr;
    };
}
