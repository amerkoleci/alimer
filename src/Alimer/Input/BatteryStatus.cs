// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

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
