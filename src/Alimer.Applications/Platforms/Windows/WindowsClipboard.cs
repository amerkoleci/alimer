// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Windows.ApplicationModel.DataTransfer;
using WindowsClipboard = Windows.ApplicationModel.DataTransfer.Clipboard;

namespace Alimer.Input;

partial class ClipboardImplementation : IClipboard
{
    public bool HasText => WindowsClipboard.GetContent().Contains(StandardDataFormats.Text);

    public Task<string?> GetTextAsync()
    {
        var clipboardContent = WindowsClipboard.GetContent();
        return clipboardContent.Contains(StandardDataFormats.Text)
            ? clipboardContent.GetTextAsync().AsTask()
            : Task.FromResult<string?>(null);
    }

    public Task SetTextAsync(string? text)
    {
        var dataPackage = new DataPackage();
        dataPackage.SetText(text);
        WindowsClipboard.SetContent(dataPackage);
        return Task.CompletedTask;
    }
}
