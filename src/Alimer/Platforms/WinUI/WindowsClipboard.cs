// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Windows.ApplicationModel.DataTransfer;
using WindowsClipboard = Windows.ApplicationModel.DataTransfer.Clipboard;

namespace Alimer.Input;

partial class Clipboard
{
    public static bool PlatformHasText()
    {
        return WindowsClipboard.GetContent().Contains(StandardDataFormats.Text);
    }

    public static Task<string?> PlatformGetTextAsync()
    {
        DataPackageView clipboardContent = WindowsClipboard.GetContent();
        return clipboardContent.Contains(StandardDataFormats.Text)
            ? clipboardContent.GetTextAsync().AsTask()
            : Task.FromResult<string?>(null);
    }

    public static Task PlatformSetTextAsync(string? text)
    {
        DataPackage dataPackage = new();
        dataPackage.SetText(text);
        WindowsClipboard.SetContent(dataPackage);
        return Task.CompletedTask;
    }
}
