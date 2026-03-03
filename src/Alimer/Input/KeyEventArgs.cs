// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public readonly struct KeyEventArgs
{
    /// <summary>
    /// Gets the key associated with this instance.
    /// </summary>
    public required Keys Key { get; init; }
}
