// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using static Alimer.AlimerApi;

namespace Alimer.Audio;

public sealed class AudioDevice : DisposableObject
{
    private float _masterVolume = 1.0f;

    public unsafe AudioDevice(in AudioDeviceOptions options)
    {
        AudioConfig config = new AudioConfig()
        {
            ListenerCount = 0,
            Channels = (uint)options.SampleChannels,
            SampleRate = (uint)options.SampleRate
        };
        if (!Alimer_AudioInit(&config))
        {
            throw new AudioException("Failed to init Audio");
        }

        //int major, minor, patch;
        //Alimer_AudioGetVersion(&major, &minor, &patch);
        //ApiVersion = new Version(major, minor, patch);

        _masterVolume = Alimer_AudioGetMasterVolume();
        OutputSampleRate = Alimer_AudioGetOutputSampleRate();
        OutputChannels = Alimer_AudioGetOutputChannels();
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="AudioDevice" /> class.
    /// </summary>
    ~AudioDevice() => Dispose(disposing: false);

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Alimer_AudioShutdown();
        }
    }

    public float MasterVolume
    {
        get => _masterVolume;
        set
        {
            Guard.IsInRange(value, 0.0f, 1.0f, nameof(value));

            _masterVolume = value;
            Alimer_AudioSetMasterVolume(value);
        }
    }

    public int OutputSampleRate { get; }
    public int OutputChannels { get; }
}
