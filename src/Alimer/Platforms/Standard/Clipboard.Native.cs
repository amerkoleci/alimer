// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer.Input;

partial class Clipboard
{
    public static partial bool HasText => alimerHasClipboardText();

    public static partial string? GetText()
    {
        return alimerClipboardGetText();
    }

    public static partial void SetText(string? text)
    {
        alimerClipboardSetText(text);
    }
}
