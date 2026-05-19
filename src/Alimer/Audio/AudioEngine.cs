// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
using System.Runtime.InteropServices;

namespace Alimer.Audio;

public sealed class AudioEngine : DisposableObject
{
    private float _masterVolume = 1.0f;

    public unsafe AudioEngine(in AudioDeviceOptions options)
    {
        // Create audio engine
        Handle = alimerAudioEngineCreate(null);
        _masterVolume = alimerAudioEngineGetMasterVolume(Handle, VolumeUnit.Linear);
        OutputChannels = alimerAudioEngineGetChannelCount(Handle);
        OutputSampleRate = alimerAudioEngineGetSampleRate(Handle);
    }

    internal AlimerApi.AudioEngine Handle { get; }

    /// <inheritdoc/>
    protected override void Dispose(bool disposing)
    {
        alimerAudioEngineDestroy(Handle);
    }

    public static AudioEngine Create(in AudioDeviceOptions options)
    {
        return new AudioEngine(options);
    }

    public float MasterVolume
    {
        get => _masterVolume;
        set
        {
            //Guard.IsInRange(value, 0.0f, 1.0f, nameof(value));

            _masterVolume = value;
            alimerAudioEngineSetMasterVolume(Handle, value, VolumeUnit.Linear);
        }
    }

    public float Volume
    {
        get => alimerAudioEngineGetVolume(Handle, VolumeUnit.Linear);
        set
        {
            alimerAudioEngineSetVolume(Handle, value, VolumeUnit.Linear);
        }
    }

    public int OutputSampleRate { get; }
    public int OutputChannels { get; }
    public AudioEngineState State => alimerAudioEngineGetState(Handle);


    public void Start()
    {
        alimerAudioEngineStart(Handle);
    }

    public void Stop()
    {
        alimerAudioEngineStop(Handle);
    }

    public float GetMasterVolume(VolumeUnit unit = VolumeUnit.Linear)
    {
        return alimerAudioEngineGetMasterVolume(Handle, unit);
    }

    public void SetMasterVolume(float value, VolumeUnit unit = VolumeUnit.Linear)
    {
        MasterVolume = value;
        alimerAudioEngineSetMasterVolume(Handle, value, unit);
    }

    public float GetVolume(VolumeUnit unit = VolumeUnit.Linear)
    {
        return alimerAudioEngineGetVolume(Handle, unit);
    }

    public void SetVolume(float value, VolumeUnit unit = VolumeUnit.Linear)
    {
        alimerAudioEngineSetVolume(Handle, value, unit);
    }

    public AudioSource CreateAudioSource(AudioClip clip)
    {
        nint handle = alimerAudioSourceCreate(Handle, clip.Handle);
        if (handle == 0)
        {
            throw new InvalidOperationException("Failed to create audio source.");
        }

        return new AudioSource(this, clip, handle);
    }
}
