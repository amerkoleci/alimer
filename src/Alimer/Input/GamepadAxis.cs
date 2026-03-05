// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// <see cref="GamepadDevice"/> axis identifiers.
/// </summary>
public enum GamepadAxis
{
    /// <summary>
    /// Left stick X axis (-1 to 1).
    /// </summary>
	LeftX,
    /// <summary>
    /// Left stick Y axis (-1 to 1).
    /// </summary>
    LeftY,
    /// <summary>
    /// Right stick X axis (-1 to 1).
    /// </summary>
    RightX,
    /// <summary>
    /// Right stick Y axis (-1 to 1).
    /// </summary>
    RightY,
    /// <summary>
    /// Left trigger (0 to 1).
    /// </summary>
    LeftTrigger,
    /// <summary>
    /// Right trigger (0 to 1).
    /// </summary>
    RightTrigger,

    Count
}
