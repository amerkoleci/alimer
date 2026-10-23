// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

public static partial class PowerStatus
{
    /// <summary>
    /// Gets the current power line status.
    /// </summary>
    public static partial PowerLineStatus Status { get; }

    /// <summary>
    /// Gets the battery status.
    /// </summary>
    /// <param name="batteryLifeTime">The number of seconds of battery life remaining, or –1 if remaining seconds are unknown or if the device is connected to AC power.</param>
    /// <param name="batteryLifePercent">The percentage of full battery charge remaining, or –1 if remaining percent is unknown or if the device is connected to AC power..</param>
    /// <returns>The <see cref="BatteryStatus"/> of the battery.</returns>
    public static partial BatteryStatus GetBatteryStatus(out int batteryLifeTime, out int batteryLifePercent);
}
