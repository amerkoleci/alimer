// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public readonly struct TextInputEventArgs
{
    /// <summary>
    /// Gets the text associated with this instance.
    /// </summary>
    public required string Text { get; init; }
}
