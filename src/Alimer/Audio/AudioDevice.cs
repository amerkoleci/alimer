// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Audio;

public abstract class AudioDevice : DisposableObject
{
    private float _masterVolume = 1.0f;

    protected  AudioDevice(in AudioDeviceOptions options)
    {
    }

    public static AudioDevice Create(in AudioDeviceOptions options)
    {
        return new OpenAL.OpenALAudioDevice(options);
    }

    public float MasterVolume
    {
        get => _masterVolume;
        set
        {
            Guard.IsInRange(value, 0.0f, 1.0f, nameof(value));

            _masterVolume = value;
            SetMasterVolume(value);
        }
    }

    public int OutputSampleRate { get; }
    public int OutputChannels { get; }

    protected abstract void SetMasterVolume(float value);
}
