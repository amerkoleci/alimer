// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Utilities.UnsafeUtilities;
using static MiniAudioNative;
using static MiniAudioNative.ma_result;

namespace Alimer.Audio.MiniAudio;

internal unsafe class MiniAudioContext : AudioContext
{
    private MiniAudioDevice[] _playbackDevices = [];
    private MiniAudioDevice[] _captureDevices = [];

    static MiniAudioContext()
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

    public MiniAudioContext()
    {
        Context = ma_ex_context_alloc();
        if (Context.IsNull)
        {
            throw new InvalidOperationException("Failed to initialize Alimer audio.");
        }

        ma_result result = ma_ex_context_init_default(Context);
        if (result != MA_SUCCESS)
        {
            string description = ma_result_description(result);
            throw new AudioException($"ma_context_init failed: {description}");
        }

        ScanDevices();
    }

    public ma_context Context { get; private set; }

    /// <inheritdoc />
    protected internal override ReadOnlySpan<AudioDevice> PlaybackDevices => _playbackDevices;

    /// <inheritdoc />
    protected internal override ReadOnlySpan<AudioDevice> CaptureDevices => _captureDevices;

    /// <inheritdoc />
    protected override void DisposeUnmanagedResources()
    {
        ma_result result = ma_context_uninit(Context);
        if (result != MA_SUCCESS)
        {
            Log.Error($"ma_context_uninit failed: {ma_result_description(result)}");
        }

        ma_free(Context);
        Context = default;
    }

    /// <inheritdoc />
    public override void ScanDevices()
    {
        ma_device_info* pPlaybackInfos;
        uint playbackCount;
        ma_device_info* pCaptureInfos;
        uint captureCount;
        ma_result result = ma_context_get_devices(Context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);
        if (result != MA_SUCCESS)
        {
            string description = ma_result_description(result);
            throw new AudioException($"ma_context_get_devices failed: {description}");
        }

        _playbackDevices = new MiniAudioDevice[playbackCount];
        for (uint i = 0; i < playbackCount; i++)
        {
            _playbackDevices[i] = new MiniAudioDevice(AudioDeviceType.Playback, &pPlaybackInfos[i]);
        }

        _captureDevices = new MiniAudioDevice[captureCount];
        for (uint i = 0; i < captureCount; i++)
        {
            _captureDevices[i] = new MiniAudioDevice(AudioDeviceType.Capture, &pCaptureInfos[i]);
        }
    }

    /// <inheritdoc />
    public override AudioEngine CreateEngine(in AudioEngineOptions options) => new MiniAudioEngine(this, in options);
}
