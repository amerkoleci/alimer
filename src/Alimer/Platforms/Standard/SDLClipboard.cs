// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Input;

partial class Clipboard
{
    public static bool PlatformHasText()
    {
        return alimerHasClipboardText();
    }

    public static Task<string?> PlatformGetTextAsync()
    {
        return Task.FromResult(alimerClipboardGetText());
    }

    public static Task PlatformSetTextAsync(string? text)
    {
        alimerClipboardSetText(text);
        return Task.CompletedTask;
    }
}
