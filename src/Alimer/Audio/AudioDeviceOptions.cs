// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Structure that describes options the <see cref="AudioEngine"/>.
/// </summary>
public record struct AudioDeviceOptions
{
    public AudioDeviceOptions()
    {
    }

    /// <summary>
    /// The sample rate to which all sources will be resampled to (if required). 
    /// Then coverts to the maximum supported sample rate by the device.
    /// </summary>
    public int SampleRate { get; set; } = 44100;

    /// <summary>
    /// The number of channels that this device is sammpling
    /// </summary>
    public int SampleChannels { get; set; } = 2;
}
