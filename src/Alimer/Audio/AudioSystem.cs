// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;

namespace Alimer.Audio;

public static class AudioSystem
{
    private static AudioContext s_context;
    private static AudioAdapter[] s_playbackAdapters = [];
    private static AudioAdapter[] s_captureAdapters = [];

    internal static AudioContext Context => s_context;

    public static unsafe void ScanDevices()
    {
        // Re-enumerate devices
        EnumAdaptersCallbackData data = new();
        GCHandle callbackHandle = GCHandle.Alloc(data);
        alimerAudioContextEnumerateDevices(
            s_context,
            & EnumeratePlaybackDevicesCallback,
            GCHandle.ToIntPtr(callbackHandle)
            );
        callbackHandle.Free();
        s_playbackAdapters = [.. data.PlaybackAdapters];
        s_captureAdapters = [.. data.CaptureAdapters];
    }

    internal static void Shutdown()
    {
        alimerAudioContextRelease(s_context);
        s_context = default;
    }

    /// <summary>
    /// Gets the list of available playback <see cref="AudioAdapter"/>.
    /// </summary>
    public static ReadOnlySpan<AudioAdapter> PlaybackAdapters => s_playbackAdapters;

    /// <summary>
    /// Gets the list of available capture <see cref="AudioAdapter"/>.
    /// </summary>
    public static ReadOnlySpan<AudioAdapter> CaptureAdapters => s_captureAdapters;

    [UnmanagedCallersOnly]
    private static void EnumeratePlaybackDevicesCallback(AudioDevice device, nint userdata)
    {
        // Handle enumerated devices
        EnumAdaptersCallbackData data = (EnumAdaptersCallbackData)GCHandle.FromIntPtr(userdata).Target!;
        AudioDeviceType deviceType = alimerAudioDeviceGetType(device);
        if (deviceType == AudioDeviceType.Playback)
        {
            data.PlaybackAdapters.Add(new AudioAdapter(device, AudioDeviceType.Playback));
        }
        else if (deviceType == AudioDeviceType.Capture)
        {
            data.CaptureAdapters.Add(new AudioAdapter(device, AudioDeviceType.Capture));
        }
    }


    private class EnumAdaptersCallbackData
    {
        public List<AudioAdapter> PlaybackAdapters = [];
        public List<AudioAdapter> CaptureAdapters = [];
    }

#pragma warning disable CA2255
    [ModuleInitializer]
    public static void Register()
    {
        s_context = alimerContextCreate(new AudioContextConfig { noAudio = false });
        if (s_context.IsNull)
        {
            throw new InvalidOperationException("Failed to initialize Alimer audio.");
        }

        ScanDevices();
    }
#pragma warning restore CA2255
}
