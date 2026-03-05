// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Represents the current PowerLine status of a device's battery.
/// </summary>
public enum PowerLineStatus
{
    Unknown,
    Online,
    Offline
}

/// <summary>
/// Represents the current current status of a device's battery.
/// </summary>
public enum BatteryStatus
{
    /// <summary>
    /// Status is unknown.
    /// </summary>
    Unknown,
    /// <summary>
    /// The battery or battery controller is not present.
    /// </summary>
    NotPresent,
    /// <summary>
    /// Not plugged in, running on the battery
    /// </summary>
    OnBattery,
    /// <summary>
    /// The battery is charging.
    /// </summary>
    Charging,
    /// <summary>
    /// The battery is charged.
    /// </summary>
    Charged,
}

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
