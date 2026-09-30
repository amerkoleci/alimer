// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

public abstract class AudioDevice
{
    protected AudioDevice()
    {
    }

    public abstract string Name { get; }
    public abstract AudioDeviceType Type { get; }
    public abstract bool IsDefault { get; }

    /// <summary>
    /// Gets the list of available playback <see cref="AudioDevice"/>.
    /// </summary>
    public static ReadOnlySpan<AudioDevice> PlaybackDevices => AudioContext.Current.PlaybackDevices;

    /// <summary>
    /// Gets the list of available capture <see cref="AudioDevice"/>.
    /// </summary>
    public static ReadOnlySpan<AudioDevice> CaptureDevices => AudioContext.Current.CaptureDevices;

    /// <summary>
    /// Gets the default playback <see cref="AudioDevice"/>. 
    /// </summary>
    public static AudioDevice DefaultPlaybackDevice
    {
        get
        {
            foreach (AudioDevice device in PlaybackDevices)
            {
                if (device.IsDefault)
                {
                    return device;
                }
            }

            throw new AudioException("No default playback device found.");
        }
    }

    /// <summary>
    /// Gets the default capture <see cref="AudioDevice"/>. 
    /// </summary>
    public static AudioDevice DefaultCaptureDevice
    {
        get
        {
            foreach (AudioDevice device in CaptureDevices)
            {
                if (device.IsDefault)
                {
                    return device;
                }
            }

            throw new AudioException("No default capture device found.");
        }
    }

    public override string ToString() => $"{Name} ({Type})";
}
