// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Audio.MiniAudio;
using static MiniAudioNative;
using static MiniAudioNative.ma_result;
using static Alimer.Utilities.UnsafeUtilities;

namespace Alimer.Audio;

public static unsafe class AudioSystem
{
    private static ma_context* s_context;
    private static AudioAdapter[] s_playbackAdapters = [];
    private static AudioAdapter[] s_captureAdapters = [];

    internal static ma_context* Context => s_context;

    static AudioSystem()
    {
        uint sizeOfDeviceId = (uint)ma_device_id_sizeof();
        uint sizeOfDeviceIdCSharp = SizeOf<ma_device_id>();
        if (sizeOfDeviceId != sizeOfDeviceIdCSharp)
        {
            throw new InvalidOperationException($"Size of ma_device_id in C# ({sizeOfDeviceIdCSharp}) does not match the size in native code ({sizeOfDeviceId}).");
        }

        uint sizeOfDeviceInfo = (uint)ma_device_info_sizeof();
        uint sizeOfDeviceInfoCSharp = SizeOf<ma_device_info>();
        if (sizeOfDeviceInfo != sizeOfDeviceInfoCSharp)
        {
            throw new InvalidOperationException($"Size of ma_device_info in C# ({sizeOfDeviceInfoCSharp}) does not match the size in native code ({sizeOfDeviceInfo}).");
        }
    }

    public static unsafe void ScanDevices()
    {
        // Re-enumerate devices
        EnumAdaptersCallbackData data = new();
        GCHandle callbackHandle = GCHandle.Alloc(data);
        ma_result result = ma_context_enumerate_devices(s_context, &EnumerateDevicesCallback, GCHandle.ToIntPtr(callbackHandle));
        if (result != MA_SUCCESS)
        {
            string description = ma_result_description(result);
            throw new AudioException($"ma_context_enumerate_devices failed: {description}");
        }

        callbackHandle.Free();
        s_playbackAdapters = [.. data.PlaybackAdapters];
        s_captureAdapters = [.. data.CaptureAdapters];
    }

    internal static void Shutdown()
    {
        ma_result result = ma_context_uninit(s_context);
        if (result != MA_SUCCESS)
        {
            Log.Error($"ma_context_uninit failed: {ma_result_description(result)}");
        }

        ma_free(s_context);
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
    private static ma_bool32 EnumerateDevicesCallback(ma_context context, ma_device_type deviceType, ma_device_info* pInfo, nint userData)
    {
        // Handle enumerated devices
        EnumAdaptersCallbackData data = (EnumAdaptersCallbackData)GCHandle.FromIntPtr(userData).Target!;
        MiniAudioAdapter adapter = new(deviceType, pInfo);
        if (adapter.Type == AudioAdapterType.Playback)
        {
            data.PlaybackAdapters.Add(adapter);
        }
        else if (adapter.Type == AudioAdapterType.Capture)
        {
            data.CaptureAdapters.Add(adapter);
        }

        return true;
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
        s_context = ma_ex_context_alloc();
        if (s_context is null)
        {
            throw new InvalidOperationException("Failed to initialize Alimer audio.");
        }

        ma_result result = ma_ex_context_init_default(s_context);
        if (result != MA_SUCCESS)
        {
            string description = ma_result_description(result);
            throw new AudioException($"ma_context_init failed: {description}");
        }


        ScanDevices();
    }
#pragma warning restore CA2255
}
