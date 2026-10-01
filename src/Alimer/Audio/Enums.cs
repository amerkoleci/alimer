// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Specifies the type of an <see cref="AudioDevice"/>.
/// </summary>
public enum AudioDeviceType
{
    /// <summary>
    /// Specifies a playback audio adapter.
    /// </summary>
    Playback,
    /// <summary>
    /// Specifies a capture audio adapter.
    /// </summary>
    Capture,
}

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

/// <summary>Current state (playing, paused, or stopped) of an <see cref="AudioSource"/>.</summary>
public enum AudioSourceState
{
    /// <summary>
    /// The <see cref="AudioSource"/> is stopped.
    /// </summary>
    Stopped,
    /// <summary>
    /// The <see cref="AudioSource"/> is playing.
    /// </summary>
    Playing,
    /// <summary>
    /// The <see cref="AudioSource"/> is paused.
    /// </summary>
    Paused,
}

public enum AudioFormat
{
    Unknown,
    Unsigned8 = 1,
    Signed16 = 2,
    Signed24 = 3,
    Signed32 = 4,
    Float32
}

public enum AudioPanMode
{
    Balance,
    Pan,
}

public enum AudioPositioning
{
    Absolute,
    Relative,
}

public enum AudioAttenuationModel
{
    None,
    Inverse,
    Linear,
    Exponential,
}

public static class AudioFormatExtensions
{
    public static uint GetSampleSize(this AudioFormat format)
    {
        return format switch
        {
            AudioFormat.Unknown => 0,
            AudioFormat.Unsigned8 => 1,
            AudioFormat.Signed16 => 2,
            AudioFormat.Signed24 => 3,
            AudioFormat.Signed32 => 4,
            AudioFormat.Float32 => 4,
            _ => 0
        };
    }
}
