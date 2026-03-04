// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public readonly struct PointerEventArgs
{
    /// <summary>
    /// Gets the current pointer position associated with this instance.
    /// </summary>
    public required PointerPoint CurrentPoint { get; init; }

    /// <summary>
    /// Gets the key modifiers with this instance.
    /// </summary>
    public required KeyModifiers KeyModifiers { get; init; }
}
