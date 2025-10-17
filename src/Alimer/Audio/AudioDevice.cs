// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;
using CommunityToolkit.Diagnostics;
using System.Runtime.InteropServices;

namespace Alimer.Audio;

public sealed class AudioDevice : DisposableObject
{
    private readonly AudioContext _context;
    private readonly AudioAdapter[] _playbackAdapters;
    private readonly AudioAdapter[] _captureAdapters;
    private readonly AudioEngine _engine;
    private float _masterVolume = 1.0f;

    private unsafe AudioDevice(in AudioDeviceOptions options)
    {
        _context = alimerAudioContextInit();

        // Create Audio context and enumerate devices
        EnumAdaptersCallbackData data = new();
        GCHandle callbackHandle = GCHandle.Alloc(data);
        alimerAudioContextEnumerateDevices(_context,
            &EnumeratePlaybackDevicesCallback,
            GCHandle.ToIntPtr(callbackHandle)
            );
        callbackHandle.Free();
        _playbackAdapters = [.. data.PlaybackAdapters];
        _captureAdapters = [.. data.CaptureAdapters];

        // Create audio engine
        _engine = alimerAudioEngineCreate(_context, null);
        _masterVolume = alimerAudioEngineGetMasterVolume(_engine, VolumeUnit.Linear);
        OutputChannels = alimerAudioEngineGetChannelCount(_engine);
        OutputSampleRate = alimerAudioEngineGetSampleRate(_engine);
    }

    /// <summary>
    /// Gets the list of available playback <see cref="AudioAdapter"/>.
    /// </summary>
    public ReadOnlySpan<AudioAdapter> PlaybackAdapters => _playbackAdapters;

    /// <summary>
    /// Gets the list of available capture <see cref="AudioAdapter"/>.
    /// </summary>
    public ReadOnlySpan<AudioAdapter> CaptureAdapters => _captureAdapters;

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            alimerAudioEngineDestroy(_engine);
            alimerAudioContextDestroy(_context);
        }
    }

    public static AudioDevice Create(in AudioDeviceOptions options)
    {
        return new AudioDevice(options);
    }

    public float MasterVolume
    {
        get => _masterVolume;
        set
        {
            Guard.IsInRange(value, 0.0f, 1.0f, nameof(value));

            _masterVolume = value;
            alimerAudioEngineSetMasterVolume(_engine, value, VolumeUnit.Linear);
        }
    }

    public int OutputSampleRate { get; }
    public int OutputChannels { get; }

    [UnmanagedCallersOnly]
    private static unsafe void EnumeratePlaybackDevicesCallback(AlimerApi.AudioDevice* device, nint userdata)
    {
        // Handle enumerated devices
        EnumAdaptersCallbackData data = (EnumAdaptersCallbackData)GCHandle.FromIntPtr(userdata).Target!;
        if (device->deviceType == AudioDeviceType.Playback)
        {
            data.PlaybackAdapters.Add(new AudioAdapter(device, AudioDeviceType.Playback));
        }
        else if (device->deviceType == AudioDeviceType.Capture)
        {
            data.CaptureAdapters.Add(new AudioAdapter(device, AudioDeviceType.Capture));
        }
    }

    //protected abstract void SetMasterVolume(float value);

    private class EnumAdaptersCallbackData
    {
        public List<AudioAdapter> PlaybackAdapters = [];
        public List<AudioAdapter> CaptureAdapters = [];
    }
}
