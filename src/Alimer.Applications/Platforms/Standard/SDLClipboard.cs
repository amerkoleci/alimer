// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Input;

partial class ClipboardImplementation : IClipboard
{
    public bool HasText => alimerHasClipboardText();

    public Task<string?> GetTextAsync()
    {
        return Task.FromResult(alimerClipboardGetText());
    }

    public Task SetTextAsync(string? text)
    {
        alimerClipboardSetText(text);
        return Task.CompletedTask;
    }
}
