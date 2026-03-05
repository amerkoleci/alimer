// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public abstract class GamepadDevice
{
    /// <summary>
    /// Gets whether the gamepad is currently connected.
    /// </summary>
	public abstract bool IsConnected { get; }

    /// <summary>
    /// Gets a value that indicates the wireless state of the gamepad.
    /// </summary>
    public abstract bool IsWireless { get;  }

    /// <summary>
    /// Returns true if the button is currently held down.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
	public abstract bool IsButtonDown(GamepadButton button);

    /// <summary>
    /// Returns true if the button was just pressed this frame.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    public abstract bool IsButtonPressed(GamepadButton button);

    /// <summary>
    /// Returns true if the button was just released this frame.
    /// </summary>
    /// <param name="button"></param>
    /// <returns></returns>
    public abstract bool IsButtonReleased(GamepadButton button);

    /// <summary>
    /// Gets the axis value (-1 to 1 for sticks, 0 to 1 for triggers).
    /// </summary>
    /// <param name="axis"></param>
    /// <returns></returns>
    public abstract float GetAxis(GamepadAxis axis);

    /// <summary>
    /// Start a rumble effect on a gamepad.
    /// </summary>
    /// <param name="leftMotor">The level of the left vibration motor. Valid values are between 0.0 and 1.0, where 0.0 signifies no motor use and 1.0 signifies max vibration.</param>
    /// <param name="rightMotor">The level of the right vibration motor. Valid values are between 0.0 and 1.0, where 0.0 signifies no motor use and 1.0 signifies max vibration.</param>
    /// <param name="durationMs">The duration of the rumble effect, in milliseconds./param>
    /// <returns>Returns true on success or false on failure</returns>
    public abstract bool Rumble(float leftMotor, float rightMotor, uint durationMs);

    /// <summary>
    /// Gets the gamepad battery status.
    /// </summary>
    /// <param name="batteryLifePercent">The percentage of full battery charge remaining, or –1 if remaining percent is unknown or if the device is connected to AC power..</param>
    /// <returns>The <see cref="BatteryStatus"/> of the battery.</returns>
    public abstract BatteryStatus GetBatteryStatus(out int batteryLifePercent);

    public abstract void Update();
}
