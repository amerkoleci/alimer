// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

public abstract class AudioEngine : DisposableObject
{
    /// <summary>
    /// Gets the sample rate of the audio engine in Hertz (Hz).
    /// </summary>
    public abstract uint SampleRate { get; }

    /// <summary>
    /// Gets the number of channels of the audio engine.
    /// </summary>
    public abstract uint Channels { get; }

    /// <summary>
    /// Gets or sets the master volume of the audio engine.
    /// </summary>
    public abstract float MasterVolume { get; set; }

    /// <summary>
    /// Gets or sets the volume of the audio engine.
    /// </summary>
    public abstract float Volume { get; set; }

    /// <summary>
    /// Get or sets the gain of the audio engine in decibels (dB).
    /// </summary>
    public abstract float GainDb { get; set; }
    
    /// <summary>
    /// Gets the state of the audio device.
    /// </summary>
    public abstract AudioDeviceState State { get; }

    public abstract ulong TimeInPcmFrames { get; }
    public abstract TimeSpan Time { get; }

    public abstract void Start();
    public abstract void Stop();
}
