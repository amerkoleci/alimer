// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public enum GamepadButton
{
    /// <summary>
    /// Bottom face button (A on Xbox, Cross on PlayStation).
    /// </summary>
	South,
    /// <summary>
    /// /// Right face button (B on Xbox, Circle on PlayStation).
    /// </summary>
    East,
    /// <summary>
    /// Left face button (X on Xbox, Square on PlayStation).
    /// </summary>
    West,
    /// <summary>
    /// Top face button (Y on Xbox, Triangle on PlayStation).
    /// </summary>
    North,
    /// <summary>
    /// Back/Select button.
    /// </summary>
    Back,
    /// <summary>
    /// Guide/Home button.
    /// </summary>
    Guide,
    /// <summary>
    /// Start button.
    /// </summary>
    Start,
    /// <summary>
    /// Left stick click.
    /// </summary>
    LeftStick,
    /// <summary>
    /// Right stick click.
    /// </summary>
    RightStick,
    /// <summary>
    /// Left shoulder/bumper.
    /// </summary>
    LeftShoulder,
    /// <summary>
    /// Right shoulder/bumper.
    /// </summary>
    RightShoulder,
    /// <summary>
    /// D-pad up.
    /// </summary>
    DPadUp,
    /// <summary>
    /// D-pad down.
    /// </summary>
    DPadDown,
    /// <summary>
    /// D-pad left.
    /// </summary>
    DPadLeft,
    /// <summary>
    /// D-pad right.
    /// </summary>
    DPadRight,
    /// <summary>
    /// Misc button (Xbox Series X share, PS5 mic, Nintendo Switch capture).
    /// </summary>
    Misc1,
    /// <summary>
    /// Additional paddle buttons.
    /// </summary>
    RightPaddle1,
    RightPaddle2,
    LeftPaddle1,
    LeftPaddle2,
    /// <summary>
    /// Touchpad button (PS4/PS5).
    /// </summary>
    Touchpad,

    Count
}
