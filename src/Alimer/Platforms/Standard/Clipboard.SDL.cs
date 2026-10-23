// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static SDL3;

namespace Alimer.Input;

partial class Clipboard
{
    public static partial bool HasText => SDL_HasClipboardText();

    public static partial Task<string?> GetTextAsync()
    {
        return Task.FromResult(SDL_GetClipboardText());
    }

    public static partial Task SetTextAsync(string? text)
    {
        SDL_SetClipboardText(text ?? string.Empty);
        return Task.CompletedTask;
    }
}
