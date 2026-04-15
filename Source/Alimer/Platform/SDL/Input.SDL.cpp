// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/Platform/Window.h"
#include "Alimer/Input.h"
#include <SDL3/SDL.h>

using namespace Alimer;

/* Input */
void Input::SetMousePosition(const Vector2& position)
{
    currentState.mouse.position = position;
    SDL_WarpMouseGlobal(position.x, position.y);
}

bool Input::HasScreenKeyboardSupport()
{
    return SDL_HasScreenKeyboardSupport();
}

std::string Input::GetClipboardText()
{
    const char* text = SDL_GetClipboardText();
    if (text)
    {
        return text;
    }
    return {};
}

void Input::SetClipboardText(std::string_view text)
{
    SDL_SetClipboardText(text.data());
}
