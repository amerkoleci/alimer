// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using SDL;
using static SDL.SDL;
using static SDL.SDL_bool;

namespace Alimer.Input;

partial class ClipboardImplementation : IClipboard
{
    public bool HasText => SDL_HasClipboardText() == SDL_TRUE;

    public Task<string?> GetTextAsync()
    {
        return Task.FromResult<string?>(SDL_GetClipboardText());
    }

    public Task SetTextAsync(string? text)
    {
        SDL_SetClipboardText(text ?? string.Empty);
        return Task.CompletedTask;
    }
}
