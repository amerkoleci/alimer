// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Platform/Clipboard.h"
#include <SDL3/SDL.h>

using namespace Alimer;

std::string Clipboard::GetText()
{
    char* text = SDL_GetClipboardText();
    if (text)
    {
        std::string result = text;
        // must free according to docs
        SDL_free(text);
        return result;
    }

    return {};
}

void Clipboard::SetText(const std::string& value)
{
    SDL_SetClipboardText(value.c_str());
}
