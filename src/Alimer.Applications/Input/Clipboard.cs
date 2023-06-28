// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// Main clipboard class to interact with the clipboard.
/// </summary>
public static class Clipboard
{
    /// <summary>
    /// Gets a value indicating whether there is any text on the clipboard.
    /// </summary>
    public static bool HasText  => Default.HasText;

    /// <summary>
    /// Returns any text that is on the clipboard.
    /// </summary>
    /// <returns>Text content that is on the clipboard, or <see langword="null"/> if there is none.</returns>
    public static Task<string?> GetTextAsync() => Default.GetTextAsync();

    /// <summary>
    /// Sets the contents of the clipboard to be the specified text.
    /// </summary>
    /// <param name="text">The text to put on the clipboard.</param>
    /// <returns>A <see cref="Task"/> object with the current status of the asynchronous operation.</returns>
    /// <remarks>This method returns immediately and does not guarentee that the text is on the clipboard by the time this method returns.</remarks>
    public static Task SetTextAsync(string? text) => Default.SetTextAsync(text ?? string.Empty);

    private static IClipboard? _defaultImplementation;

    /// <summary>
    /// Provides the default implementation for static usage of this API.
    /// </summary>
    internal static IClipboard Default => _defaultImplementation ??= new ClipboardImplementation();
}

interface IClipboard
{
    bool HasText { get; }

    Task<string?> GetTextAsync();
    Task SetTextAsync(string? text);
}

partial class ClipboardImplementation : IClipboard
{
}
