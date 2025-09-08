// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Android.Content;
using static Android.Content.ClipboardManager;
using AndroidApplication = Android.App.Application;

namespace Alimer.Input;

partial class ClipboardImplementation : IClipboard
{
    private static ClipboardManager? _clipboardManager;

    private static ClipboardManager? ClipboardManager => _clipboardManager ??= AndroidApplication.Context.GetSystemService(Context.ClipboardService) as ClipboardManager;

    public bool HasText
    {
        get
        {
            return
                ClipboardManager is not null &&
                ClipboardManager.HasPrimaryClip &&
                !string.IsNullOrEmpty(ClipboardManager.PrimaryClip?.GetItemAt(0)?.Text);
        }
    }

    public Task<string?> GetTextAsync()
    {
        return Task.FromResult(ClipboardManager?.PrimaryClip?.GetItemAt(0)?.Text);
    }

    public Task SetTextAsync(string? text)
    {
        if (ClipboardManager is not null)
            ClipboardManager.PrimaryClip = ClipData.NewPlainText("Text", text ?? string.Empty);

        return Task.CompletedTask;
    }
}
