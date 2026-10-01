// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static MiniAudioNative;
using static MiniAudioNative.ma_result;
using static MiniAudioNative.ma_device_state;
using Vortice.Vulkan;

namespace Alimer.Audio.MiniAudio;

internal unsafe class MiniAudioEngine : AudioEngine
{
    internal MiniAudioEngine(MiniAudioContext context, in AudioEngineOptions options)
    {
        Handle = ma_ex_engine_alloc();

        var config = new ma_engine_ex_config();

#if TODO
        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
        if (config && config->playbackDevice)
        {
            deviceConfig.playback.pDeviceID = &config->playbackDevice->info->id;
        }
        deviceConfig.playback.format = ma_format_f32;
        deviceConfig.playback.channels = (config != nullptr && config->channelCount > 0) ? config->channelCount : 2;
        deviceConfig.sampleRate = (config != nullptr && config->sampleRate > 0) ? config->sampleRate : 48000;
        deviceConfig.dataCallback = DataCallback;
        deviceConfig.pUserData = engine;

        ma_result result = ma_device_init(context->handle, &deviceConfig, &engine->device);
        if (result != MA_SUCCESS)
        {
            alimerLogError(LogCategory_Audio, "Failed to initialize audio device");
            delete engine;
            return nullptr;
        }

        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.pDevice = &engine->device;
        engineConfig.pProcessUserData = engine;
        engineConfig.listenerCount = 1;
        result = ma_engine_init(&engineConfig, &engine->handle); 
#endif

        ma_result result = ma_ex_engine_init_default(Handle); // ma_ex_engine_init_with_config(context.Handle, &config, Handle);
        if (result != MA_SUCCESS)
        {
            string description = ma_result_description(result);
            throw new AudioException($"ma_engine_init failed: {description}");
        }

        Device = ma_engine_get_device(Handle);

        EndpointNode = ma_engine_get_endpoint(Handle);
        NodeGraph = ma_engine_get_node_graph(Handle);
        ListenerCount = ma_engine_get_listener_count(Handle);
    }

    public ma_engine* Handle { get; private set; }
    public ma_device* Device { get; }
    public ma_node* EndpointNode { get; }
    public ma_node_graph* NodeGraph { get; }
    public uint ListenerCount { get; }

    /// <inheritdoc />
    public override uint SampleRate => ma_engine_get_sample_rate(Handle);

    /// <inheritdoc />
    public override uint Channels => ma_engine_get_channels(Handle);

    /// <inheritdoc />
    public override float MasterVolume
    {
        get
        {
            float volume;
            ma_device_get_master_volume(Device, &volume).CheckResult(nameof(ma_device_get_master_volume));
            return volume;
        }
        set
        {
            ma_device_set_master_volume(Device, value).CheckResult(nameof(ma_device_set_master_volume));
        }
    }

    /// <inheritdoc />
    public override float MasterGainDb
    {
        get
        {
            float gainDB;
            ma_device_get_master_volume_db(Device, &gainDB).CheckResult(nameof(ma_device_get_master_volume_db));
            return gainDB;
        }
        set
        {
            ma_device_set_master_volume_db(Device, value).CheckResult(nameof(ma_device_set_master_volume_db));
        }
    }

    /// <inheritdoc />
    public override float Volume
    {
        get => ma_engine_get_volume(Handle);
        set => ma_engine_set_volume(Handle, value);
    }

    /// <inheritdoc />
    public override float GainDb
    {
        get => ma_engine_get_gain_db(Handle);
        set => ma_engine_set_gain_db(Handle, value);
    }

    /// <inheritdoc />
    public override AudioDeviceState State
    {
        get
        {
            ma_device_state state = ma_device_get_state(Device);
            return state switch
            {
                ma_device_state_uninitialized => AudioDeviceState.Uninitialized,
                ma_device_state_stopped => AudioDeviceState.Stopped,
                ma_device_state_started => AudioDeviceState.Started,
                ma_device_state_starting => AudioDeviceState.Starting,
                ma_device_state_stopping => AudioDeviceState.Stopping,
                _ => throw new AudioException($"Unknown device state: {state}"),
            };
        }
    }

    /// <inheritdoc />
    public override ulong TimeInPcmFrames
    {
        get => ma_engine_get_time_in_pcm_frames(Handle);
        set => ma_engine_set_time_in_pcm_frames(Handle, value);
    }

    public override TimeSpan Time
    {
        get
        {
            ThrowIfDisposed();
            ulong milliseconds = ma_engine_get_time_in_milliseconds(Handle);
            return TimeSpan.FromMilliseconds(milliseconds);
        }
        set
        {
            ThrowIfDisposed();
            double totalMilliseconds = Math.Round(value.TotalMilliseconds, MidpointRounding.AwayFromZero);
            double clampedMilliseconds = Math.Clamp(totalMilliseconds, 0d, ulong.MaxValue);
            ma_engine_set_time_in_milliseconds(Handle, (ulong)clampedMilliseconds).CheckResult(nameof(ma_engine_set_time_in_milliseconds));
        }
    }

    /// <inheritdoc />
    protected override void DisposeUnmanagedResources()
    {
        ma_engine_uninit(Handle);
        ma_free(Handle);
        Handle = default;
    }

    /// <inheritdoc />
    public override void Start()
    {
        ThrowIfDisposed();
        ma_engine_start(Handle).CheckResult(nameof(ma_engine_start));
    }

    /// <inheritdoc />
    public override void Stop()
    {
        ma_engine_stop(Handle).CheckResult(nameof(ma_engine_stop));
    }

    /// <inheritdoc />
    public override AudioSource CreateAudioSource() => new MiniAudioSource(this);
}
