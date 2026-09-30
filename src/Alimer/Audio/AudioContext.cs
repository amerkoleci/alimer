// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Represents the audio context for managing audio devices and creating audio engines.
/// </summary>
public abstract class AudioContext : DisposableObject
{
    /// <summary>
    /// Gets or sets the current <see cref="AudioContext"/> instance.
    /// </summary>
    public static AudioContext Current
    {
        get
        {
            field ??= new MiniAudio.MiniAudioContext();

            return field;
        }
        set;
    }

    /// <summary>
    /// Scans for available audio devices and updates the list of playback and capture adapters.
    /// </summary>
    public abstract void ScanDevices();

    /// <summary>
    /// Creates a new <see cref="AudioEngine"/> instance with the default options.
    /// </summary>
    /// <returns>The created <see cref="AudioEngine"/> instance.</returns>
    public AudioEngine CreateEngine() => CreateEngine(new AudioEngineOptions());

    /// <summary>
    /// Creates a new <see cref="AudioEngine"/> instance with the specified options.
    /// </summary>
    /// <param name="options">The options to use when creating the audio engine.</param>
    /// <returns>The created <see cref="AudioEngine"/> instance.</returns>
    public abstract AudioEngine CreateEngine(in AudioEngineOptions options);

    /// <summary>
    /// Gets the list of available playback <see cref="AudioDevice"/>.
    /// </summary>
    protected internal abstract ReadOnlySpan<AudioDevice> PlaybackDevices { get; }

    /// <summary>
    /// Gets the list of available capture <see cref="AudioDevice"/>.
    /// </summary>
    protected internal abstract ReadOnlySpan<AudioDevice> CaptureDevices { get; }

}
