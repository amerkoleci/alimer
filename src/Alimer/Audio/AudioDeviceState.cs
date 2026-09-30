// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Represents the state of an audio device.
/// </summary>
public enum AudioDeviceState
{
    /// <summary>
    /// The device is uninitialized. This is the default state of a device after it has been created but before it has been initialized.
    /// </summary>
    Uninitialized,
    /// <summary>
    /// The device is stopped. In this state, the device is not requesting or delivering audio data. This is the default state of a device after it has been initialized.
    /// </summary>
    Stopped,
    /// <summary>
    /// The device is started. In this state, the device is requesting and/or delivering audio data.
    /// </summary>
    Started,
    /// <summary>
    /// The device is starting. In this state, the device is transitioning from the stopped state to the started state.
    /// </summary>
    Starting,
    /// <summary>
    /// The device is stopping. In this state, the device is transitioning from the started state to the stopped state.
    /// </summary>
    Stopping,
}
