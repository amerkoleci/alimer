// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using Alimer;
using ma_uint8 = System.Byte;
using ma_uint32 = System.UInt32;
using ma_uint64 = System.UInt64;
using ma_vec3f = System.Numerics.Vector3;
using static MiniAudioNative.ma_result;
using Alimer.Audio;
#pragma warning disable CS0649

/* Callback for when a sound reaches the end. */
using unsafe ma_sound_end_proc = delegate* unmanaged[Cdecl]<void* /*pUserData*/, MiniAudioNative.ma_sound* /*pSound*/, void>;


internal unsafe static partial class MiniAudioNative
{
    private const string LibraryName = AlimerApi.LibraryName;

    public const int MA_MAX_DEVICE_NAME_LENGTH = 255;


    [DebuggerHidden]
    [DebuggerStepThrough]
    public static void CheckResult(this ma_result result, string api)
    {
        if (result == MA_SUCCESS)
        {
            return;
        }

        string description = ma_result_description(result);
        throw new AudioException($"miniaudio API '{api}' failed with code {result}: {description}.");
    }

    #region Enums
    public enum ma_channel : ma_uint8;

    public enum ma_result
    {
        MA_SUCCESS = 0,
        MA_ERROR = -1,  /* A generic error. */
        MA_INVALID_ARGS = -2,
        MA_INVALID_OPERATION = -3,
        MA_OUT_OF_MEMORY = -4,
        MA_OUT_OF_RANGE = -5,
        MA_ACCESS_DENIED = -6,
        MA_DOES_NOT_EXIST = -7,
        MA_ALREADY_EXISTS = -8,
        MA_TOO_MANY_OPEN_FILES = -9,
        MA_INVALID_FILE = -10,
        MA_TOO_BIG = -11,
        MA_PATH_TOO_LONG = -12,
        MA_NAME_TOO_LONG = -13,
        MA_NOT_DIRECTORY = -14,
        MA_IS_DIRECTORY = -15,
        MA_DIRECTORY_NOT_EMPTY = -16,
        MA_AT_END = -17,
        MA_NO_SPACE = -18,
        MA_BUSY = -19,
        MA_IO_ERROR = -20,
        MA_INTERRUPT = -21,
        MA_UNAVAILABLE = -22,
        MA_ALREADY_IN_USE = -23,
        MA_BAD_ADDRESS = -24,
        MA_BAD_SEEK = -25,
        MA_BAD_PIPE = -26,
        MA_DEADLOCK = -27,
        MA_TOO_MANY_LINKS = -28,
        MA_NOT_IMPLEMENTED = -29,
        MA_NO_MESSAGE = -30,
        MA_BAD_MESSAGE = -31,
        MA_NO_DATA_AVAILABLE = -32,
        MA_INVALID_DATA = -33,
        MA_TIMEOUT = -34,
        MA_NO_NETWORK = -35,
        MA_NOT_UNIQUE = -36,
        MA_NOT_SOCKET = -37,
        MA_NO_ADDRESS = -38,
        MA_BAD_PROTOCOL = -39,
        MA_PROTOCOL_UNAVAILABLE = -40,
        MA_PROTOCOL_NOT_SUPPORTED = -41,
        MA_PROTOCOL_FAMILY_NOT_SUPPORTED = -42,
        MA_ADDRESS_FAMILY_NOT_SUPPORTED = -43,
        MA_SOCKET_NOT_SUPPORTED = -44,
        MA_CONNECTION_RESET = -45,
        MA_ALREADY_CONNECTED = -46,
        MA_NOT_CONNECTED = -47,
        MA_CONNECTION_REFUSED = -48,
        MA_NO_HOST = -49,
        MA_IN_PROGRESS = -50,
        MA_CANCELLED = -51,
        MA_MEMORY_ALREADY_MAPPED = -52,

        /* General non-standard errors. */
        MA_CRC_MISMATCH = -100,

        /* General miniaudio-specific errors. */
        MA_FORMAT_NOT_SUPPORTED = -200,
        MA_DEVICE_TYPE_NOT_SUPPORTED = -201,
        MA_SHARE_MODE_NOT_SUPPORTED = -202,
        MA_NO_BACKEND = -203,
        MA_NO_DEVICE = -204,
        MA_API_NOT_FOUND = -205,
        MA_INVALID_DEVICE_CONFIG = -206,
        MA_LOOP = -207,
        MA_BACKEND_NOT_ENABLED = -208,

        /* State errors. */
        MA_DEVICE_NOT_INITIALIZED = -300,
        MA_DEVICE_ALREADY_INITIALIZED = -301,
        MA_DEVICE_NOT_STARTED = -302,
        MA_DEVICE_NOT_STOPPED = -303,

        /* Operation errors. */
        MA_FAILED_TO_INIT_BACKEND = -400,
        MA_FAILED_TO_OPEN_BACKEND_DEVICE = -401,
        MA_FAILED_TO_START_BACKEND_DEVICE = -402,
        MA_FAILED_TO_STOP_BACKEND_DEVICE = -403
    }

    public enum ma_device_type
    {
        ma_device_type_playback = 1,
        ma_device_type_capture = 2,
        ma_device_type_duplex = ma_device_type_playback | ma_device_type_capture, /* 3 */
        ma_device_type_loopback = 4
    }

    public enum ma_device_state
    {
        ma_device_state_uninitialized = 0,
        ma_device_state_stopped = 1,  /* The device's default state after initialization. */
        ma_device_state_started = 2,  /* The device is started and is requesting and/or delivering audio data. */
        ma_device_state_starting = 3,  /* Transitioning from a stopped state to started. */
        ma_device_state_stopping = 4   /* Transitioning from a started state to stopped. */
    }

    public enum ma_format
    {
        ma_format_unknown = 0,
        ma_format_u8 = 1,
        ma_format_s16 = 2,
        ma_format_s24 = 3,
        ma_format_s32 = 4,
        ma_format_f32 = 5,
        ma_format_count
    }

    [Flags]
    public enum ma_sound_flags
    {
        /* Resource manager flags. */
        MA_SOUND_FLAG_STREAM = 0x00000001,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_STREAM */
        MA_SOUND_FLAG_DECODE = 0x00000002,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE */
        MA_SOUND_FLAG_ASYNC = 0x00000004,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC */
        MA_SOUND_FLAG_WAIT_INIT = 0x00000008,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_WAIT_INIT */
        MA_SOUND_FLAG_UNKNOWN_LENGTH = 0x00000010,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_UNKNOWN_LENGTH */
        MA_SOUND_FLAG_LOOPING = 0x00000020,   /* MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_LOOPING */

        /* ma_sound specific flags. */
        MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT = 0x00001000,   /* Do not attach to the endpoint by default. Useful for when setting up nodes in a complex graph system. */
        MA_SOUND_FLAG_NO_PITCH = 0x00002000,   /* Disable pitch shifting with ma_sound_set_pitch() and ma_sound_group_set_pitch(). This is an optimization. */
        MA_SOUND_FLAG_NO_SPATIALIZATION = 0x00004000    /* Disable spatialization. */
    }


    public enum ma_attenuation_model
    {
        ma_attenuation_model_none,          /* No distance attenuation and no spatialization. */
        ma_attenuation_model_inverse,       /* Equivalent to OpenAL's AL_INVERSE_DISTANCE_CLAMPED. */
        ma_attenuation_model_linear,        /* Linear attenuation. Equivalent to OpenAL's AL_LINEAR_DISTANCE_CLAMPED. */
        ma_attenuation_model_exponential    /* Exponential attenuation. Equivalent to OpenAL's AL_EXPONENT_DISTANCE_CLAMPED. */
    }

    public enum ma_positioning
    {
        ma_positioning_absolute,
        ma_positioning_relative
    }
    #endregion

    [StructLayout(LayoutKind.Sequential, Size = 4)]
    public readonly record struct ma_bool32 : IEquatable<ma_bool32>
    {
        public static ma_bool32 True => new(true);
        public static ma_bool32 False => new(false);

        /// <summary>
        /// Initializes a new instance of the <see cref="ma_bool32" /> class.
        /// </summary>
        /// <param name="boolValue">if set to <c>true</c> [bool value].</param>
        public ma_bool32(bool boolValue)
        {
            Value = boolValue ? 1u : 0u;
        }

        public readonly uint Value;

        /// <summary>
        /// Performs an explicit conversion from <see cref="ma_bool32"/> to <see cref="bool"/>.
        /// </summary>
        /// <param name="value">The <see cref="ma_bool32"/> value.</param>
        /// <returns>The result of the conversion.</returns>
        public static implicit operator bool(ma_bool32 value) => value.Value != 0;

        /// <summary>
        /// Performs an explicit conversion from <see cref="bool"/> to <see cref="ma_bool32"/>.
        /// </summary>
        /// <param name="boolValue">The value.</param>
        /// <returns>The result of the conversion.</returns>
        public static implicit operator ma_bool32(bool boolValue) => new(boolValue);

        /// <inheritdoc/>
        public override string ToString() => Value != 0 ? "True" : "False";
    }

    [StructLayout(LayoutKind.Explicit)]
    internal partial struct ma_device_ic_custom
    {
        [FieldOffset(0)]
        public int i;
        [FieldOffset(0)]
        public fixed byte s[256];
        [FieldOffset(0)]
        public nint p;
    }

    [StructLayout(LayoutKind.Explicit)]
    public partial struct ma_device_id
    {
        [FieldOffset(0)]
        public fixed char wasapi[64];      /* WASAPI uses a wchar_t string for identification. */

        [FieldOffset(0)]
        public fixed byte dsound[16];            /* DirectSound uses a GUID for identification. */
        [FieldOffset(0)]
        public uint winmm;   /* When creating a device, WinMM expects a Win32 UINT_PTR for device identification. In practice it's actually just a UINT. */
        [FieldOffset(0)]
        public fixed byte alsa[256];                 /* ALSA uses a name string for identification. */
        [FieldOffset(0)]
        public fixed byte pulse[256];                /* PulseAudio uses a name string for identification. */
        [FieldOffset(0)]
        public int jack;                       /* JACK always uses default devices. */
        [FieldOffset(0)]
        public fixed byte coreaudio[256];            /* Core Audio uses a string for identification. */
        [FieldOffset(0)]
        public fixed byte sndio[256];                /* "snd/0", etc. */
        [FieldOffset(0)]
        public fixed byte audio4[256];               /* "/dev/audio", etc. */
        [FieldOffset(0)]
        public fixed byte oss[64];                   /* "dev/dsp0", etc. "dev/dsp" for the default device. */
        [FieldOffset(0)]
        public int aaudio;                /* AAudio uses a 32-bit integer for identification. */
        [FieldOffset(0)]
        public uint opensl;               /* OpenSL|ES uses a 32-bit unsigned integer for identification. */
        [FieldOffset(0)]
        public fixed byte webaudio[32];              /* Web Audio always uses default devices for now, but if this changes it'll be a GUID. */
        [FieldOffset(0)]
        internal ma_device_ic_custom custom;                       /* The custom backend could be anything. Give them a few options. */
        [FieldOffset(0)]
        public int nullbackend;                /* The null backend uses an integer for device IDs. */
    }

    public struct ma_device_info_native_data_format
    {
        public ma_format format;       /* Sample format. If set to ma_format_unknown, all sample formats are supported. */
        public uint channels;     /* If set to 0, all channels are supported. */
        public uint sampleRate;   /* If set to 0, all sample rates are supported. */
        public uint flags;        /* A combination of MA_DATA_FORMAT_FLAG_* flags. */
    }

    public struct ma_device_info
    {
        /* Basic info. This is the only information guaranteed to be filled in during device enumeration. */
        public ma_device_id id;
        public fixed byte name[MA_MAX_DEVICE_NAME_LENGTH + 1];   /* +1 for null terminator. */
        public ma_bool32 isDefault;

        public uint nativeDataFormatCount;
        public ma_device_info_native_data_format__FixedBuffer nativeDataFormats;

        [InlineArray(/*ma_format_count * ma_standard_sample_rate_count * MA_MAX_CHANNELS*/ 64)]
        public partial struct ma_device_info_native_data_format__FixedBuffer
        {
            public ma_device_info_native_data_format e0;
        }
    }

    public partial struct ma_log;

    public partial struct ma_context;

    public partial struct ma_device;

    public partial struct ma_engine;

    public partial struct ma_data_source;

    public partial struct ma_sound;
    public partial struct ma_sound_group;  /* typedef ma_sound        ma_sound_group; */

    public partial struct ma_node;

    public partial struct ma_node_graph;

    [LibraryImport(LibraryName)]
    [return: MarshalUsing(typeof(UTF8OwnedMarshaler))]
    public static partial string ma_result_description(ma_result result);

    [LibraryImport(LibraryName)]
    public static partial void* ma_malloc(nuint sz, /*const ma_allocation_callbacks**/
    nint pAllocationCallbacks = 0);

    [LibraryImport(LibraryName)]
    public static partial void* ma_calloc(nuint sz, /*const ma_allocation_callbacks**/nint pAllocationCallbacks = 0);

    [LibraryImport(LibraryName)]
    public static partial void* ma_realloc(nuint sz, /*const ma_allocation_callbacks**/nint pAllocationCallbacks = 0);

    [LibraryImport(LibraryName)]
    public static partial void ma_free(void* ptr, /*const ma_allocation_callbacks**/nint pAllocationCallbacks = 0);

    public static void ma_free(nint ptr, /*const ma_allocation_callbacks**/nint pAllocationCallbacks = 0)
    {
        ma_free((void*)ptr, pAllocationCallbacks);
    }

    [LibraryImport(LibraryName)]
    public static partial nuint ma_device_id_sizeof();

    [LibraryImport(LibraryName)]
    public static partial nuint ma_context_sizeof();

    [LibraryImport(LibraryName)]
    public static partial nuint ma_device_info_sizeof();

    [LibraryImport(LibraryName)]
    public static partial nuint ma_engine_sizeof();


    [LibraryImport(LibraryName)]
    public static partial nuint ma_sound_group_sizeof();


    public static float ma_volume_linear_to_db(float factor)
    {
        return 20 * MathF.Log10(factor);
    }

    public static float ma_volume_db_to_linear(float gain)
    {
        return MathF.Pow(10, gain / 20.0f);
    }

    #region Context
    public static ma_context* ma_ex_context_alloc()
    {
        return (ma_context*)ma_malloc(ma_context_sizeof());
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_context_uninit(ma_context* pContext);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_ex_context_init_default(ma_context* result);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_context_get_devices(ma_context* context, ma_device_info** ppPlaybackDeviceInfos, ma_uint32* pPlaybackDeviceCount, ma_device_info** ppCaptureDeviceInfos, ma_uint32* pCaptureDeviceCount);

    // typedef ma_bool32 (* ma_enum_devices_callback_proc)(ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData);
    //[LibraryImport(LibraryName)]
    //public static partial ma_result ma_context_enumerate_devices(ma_context context, delegate* unmanaged<ma_context, ma_device_type, ma_device_info*, nint, ma_bool32> callback, nint userData);
    #endregion

    #region Device
    [LibraryImport(LibraryName)]
    private static partial nuint ma_device_sizeof();

    public static ma_device* ma_ex_device_alloc()
    {
        return (ma_device*)ma_malloc(ma_device_sizeof());
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_ex_device_init_default(ma_context* pContext, ma_device_type deviceType, ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial void ma_device_uninit(ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_get_info(ma_device* pDevice, ma_device_type type, ma_device_info* pDeviceInfo);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_get_name(ma_device* pDevice, ma_device_type type, byte* pName, nuint nameCap, nuint* pLengthNotIncludingNullTerminator);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_start(ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_stop(ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_device_is_started(ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial ma_device_state ma_device_get_state(ma_device* pDevice);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_set_master_volume(ma_device* pDevice, float volume);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_get_master_volume(ma_device* pDevice, float* pVolume);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_set_master_volume_db(ma_device* pDevice, float gainDB);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_device_get_master_volume_db(ma_device* pDevice, float* pGainDB);
    #endregion

    #region engine
    public static ma_engine* ma_ex_engine_alloc()
    {
        return (ma_engine*)ma_malloc(ma_engine_sizeof());
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_ex_engine_init_default(ma_engine* pEngine);

    public struct ma_engine_ex_config
    {
        public ma_device_id* playbackDeviceID;
        /// Audio output channel count.
        public uint channelCount;
        /// Audio output sample rate.
        public uint sampleRate;
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_ex_engine_init_with_config(ma_context* pContext, ma_engine_ex_config* config, ma_engine* pEngine);

    [LibraryImport(LibraryName)]
    public static partial void ma_engine_uninit(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_read_pcm_frames(ma_engine* pEngine, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);

    [LibraryImport(LibraryName)]
    public static partial ma_node_graph* ma_engine_get_node_graph(ma_engine* pEngine);

    [LibraryImport(LibraryName)]
    public static partial ma_device* ma_engine_get_device(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_log* ma_engine_get_log(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_node* ma_engine_get_endpoint(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_engine_get_time_in_pcm_frames(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_engine_get_time_in_milliseconds(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_set_time_in_pcm_frames(ma_engine* pEngine, ma_uint64 globalTime);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_set_time_in_milliseconds(ma_engine* pEngine, ma_uint64 globalTime);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_engine_get_time(ma_engine* pEngine);                  /* Deprecated. Use ma_engine_get_time_in_pcm_frames(). Will be removed in version 0.12. */
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_set_time(ma_engine* pEngine, ma_uint64 globalTime);  /* Deprecated. Use ma_engine_set_time_in_pcm_frames(). Will be removed in version 0.12. */
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_engine_get_channels(ma_engine* pEngine); // ma_node_graph_get_channels(&pEngine->nodeGraph);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_engine_get_sample_rate(ma_engine* pEngine);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_start(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_stop(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_set_volume(ma_engine* pEngine, float volume);
    [LibraryImport(LibraryName)]
    public static partial float ma_engine_get_volume(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_engine_set_gain_db(ma_engine* pEngine, float gainDB);
    [LibraryImport(LibraryName)]
    public static partial float ma_engine_get_gain_db(ma_engine* pEngine);

    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_engine_get_listener_count(ma_engine* pEngine);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_engine_find_closest_listener(ma_engine* pEngine, float absolutePosX, float absolutePosY, float absolutePosZ);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_position(ma_engine* pEngine, ma_uint32 listenerIndex, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_engine_listener_get_position(ma_engine* pEngine, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_direction(ma_engine* pEngine, ma_uint32 listenerIndex, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_engine_listener_get_direction(ma_engine* pEngine, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_velocity(ma_engine* pEngine, ma_uint32 listenerIndex, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_engine_listener_get_velocity(ma_engine* pEngine, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_cone(ma_engine* pEngine, ma_uint32 listenerIndex, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_get_cone(ma_engine* pEngine, ma_uint32 listenerIndex, float* pInnerAngleInRadians, float* pOuterAngleInRadians, float* pOuterGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_world_up(ma_engine* pEngine, ma_uint32 listenerIndex, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_engine_listener_get_world_up(ma_engine* pEngine, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial void ma_engine_listener_set_enabled(ma_engine* pEngine, ma_uint32 listenerIndex, ma_bool32 isEnabled);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_engine_listener_is_enabled(ma_engine* pEngine, ma_uint32 listenerIndex);

    #endregion

    #region NodeGraph
    //[LibraryImport(LibraryName)]
    //public static partial ma_result ma_node_graph_init(const ma_node_graph_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_node_graph*pNodeGraph);
    //[LibraryImport(LibraryName)]
    //public static partial void ma_node_graph_uninit(ma_node_graph* pNodeGraph, const ma_allocation_callbacks* pAllocationCallbacks);
    [LibraryImport(LibraryName)]
    public static partial ma_node* ma_node_graph_get_endpoint(ma_node_graph* pNodeGraph);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_node_graph_read_pcm_frames(ma_node_graph* pNodeGraph, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_node_graph_get_channels(ma_node_graph* pNodeGraph);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_node_graph_get_time(ma_node_graph* pNodeGraph);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_node_graph_set_time(ma_node_graph* pNodeGraph, ma_uint64 globalTime);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_node_graph_get_processing_size_in_frames(ma_node_graph* pNodeGraph);
    #endregion

    #region Sound

    [LibraryImport(LibraryName)]
    public static partial nuint ma_sound_sizeof();

    public static ma_sound* ma_ex_sound_alloc()
    {
        return (ma_sound*)ma_malloc(ma_sound_sizeof());
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_init_from_data_source(ma_engine* pEngine, ma_data_source* pDataSource, ma_uint32 flags, ma_sound_group* pGroup, ma_sound* pSound);

    [LibraryImport(LibraryName)]
    public static partial void ma_sound_uninit(ma_sound* pSound);

    [LibraryImport(LibraryName)]
    public static partial ma_engine* ma_sound_get_engine(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_data_source* ma_sound_get_data_source(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_start(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_stop(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_stop_with_fade_in_pcm_frames(ma_sound* pSound, ma_uint64 fadeLengthInFrames);     /* Will overwrite any scheduled stop and fade. If you want to restart the sound, first reset it with `ma_sound_reset_stop_time_and_fade()`. There are plans to make this less awkward in the future. */
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_stop_with_fade_in_milliseconds(ma_sound* pSound, ma_uint64 fadeLengthInFrames);   /* Will overwrite any scheduled stop and fade. If you want to restart the sound, first reset it with `ma_sound_reset_stop_time_and_fade()`. There are plans to make this less awkward in the future. */
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_reset_start_time(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_reset_stop_time(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_reset_fade(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_reset_stop_time_and_fade(ma_sound* pSound);  /* Resets fades and scheduled stop time. Does not seek back to the start. */
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_volume(ma_sound* pSound, float volume);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_volume(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_pan(ma_sound* pSound, float pan);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_pan(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_pan_mode(ma_sound* pSound, ma_pan_mode panMode);
    [LibraryImport(LibraryName)]
    public static partial ma_pan_mode ma_sound_get_pan_mode(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_pitch(ma_sound* pSound, float pitch);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_pitch(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_spatialization_enabled(ma_sound* pSound, ma_bool32 enabled);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_is_spatialization_enabled(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_pinned_listener_index(ma_sound* pSound, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_sound_get_pinned_listener_index(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_sound_get_listener_index(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_get_direction_to_listener(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_position(ma_sound* pSound, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_get_position(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_direction(ma_sound* pSound, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_get_direction(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_velocity(ma_sound* pSound, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_get_velocity(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_attenuation_model(ma_sound* pSound, ma_attenuation_model attenuationModel);
    [LibraryImport(LibraryName)]
    public static partial ma_attenuation_model ma_sound_get_attenuation_model(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_positioning(ma_sound* pSound, ma_positioning positioning);
    [LibraryImport(LibraryName)]
    public static partial ma_positioning ma_sound_get_positioning(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_rolloff(ma_sound* pSound, float rolloff);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_rolloff(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_min_gain(ma_sound* pSound, float minGain);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_min_gain(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_max_gain(ma_sound* pSound, float maxGain);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_max_gain(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_min_distance(ma_sound* pSound, float minDistance);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_min_distance(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_max_distance(ma_sound* pSound, float maxDistance);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_max_distance(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_cone(ma_sound* pSound, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_get_cone(ma_sound* pSound, float* pInnerAngleInRadians, float* pOuterAngleInRadians, float* pOuterGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_doppler_factor(ma_sound* pSound, float dopplerFactor);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_doppler_factor(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_directional_attenuation_factor(ma_sound* pSound, float directionalAttenuationFactor);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_directional_attenuation_factor(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_fade_in_pcm_frames(ma_sound* pSound, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_fade_in_milliseconds(ma_sound* pSound, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_fade_start_in_pcm_frames(ma_sound* pSound, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInFrames, ma_uint64 absoluteGlobalTimeInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_fade_start_in_milliseconds(ma_sound* pSound, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInMilliseconds, ma_uint64 absoluteGlobalTimeInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_get_current_fade_volume(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_start_time_in_pcm_frames(ma_sound* pSound, ma_uint64 absoluteGlobalTimeInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_start_time_in_milliseconds(ma_sound* pSound, ma_uint64 absoluteGlobalTimeInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_stop_time_in_pcm_frames(ma_sound* pSound, ma_uint64 absoluteGlobalTimeInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_stop_time_in_milliseconds(ma_sound* pSound, ma_uint64 absoluteGlobalTimeInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_stop_time_with_fade_in_pcm_frames(ma_sound* pSound, ma_uint64 stopAbsoluteGlobalTimeInFrames, ma_uint64 fadeLengthInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_stop_time_with_fade_in_milliseconds(ma_sound* pSound, ma_uint64 stopAbsoluteGlobalTimeInMilliseconds, ma_uint64 fadeLengthInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_is_playing(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_sound_get_time_in_pcm_frames(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_sound_get_time_in_milliseconds(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_set_looping(ma_sound* pSound, ma_bool32 isLooping);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_is_looping(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_at_end(ma_sound* pSound);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_seek_to_pcm_frame(ma_sound* pSound, ma_uint64 frameIndex); /* Just a wrapper around ma_data_source_seek_to_pcm_frame(). */
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_seek_to_second(ma_sound* pSound, float seekPointInSeconds); /* Abstraction to ma_sound_seek_to_pcm_frame() */
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_get_data_format(ma_sound* pSound, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, nuint channelMapCap);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_get_cursor_in_pcm_frames(ma_sound* pSound, ma_uint64* pCursor);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_get_length_in_pcm_frames(ma_sound* pSound, ma_uint64* pLength);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_get_cursor_in_seconds(ma_sound* pSound, float* pCursor);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_get_length_in_seconds(ma_sound* pSound, float* pLength);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_set_end_callback(ma_sound* pSound, ma_sound_end_proc callback, void* pUserData);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_group_init(ma_engine* pEngine, ma_uint32 flags, ma_sound_group* pParentGroup, ma_sound_group* pGroup);
    //[LibraryImport(LibraryName)]
    //public static partial ma_result ma_sound_group_init_ex(ma_engine* pEngine,  ma_sound_group_config* pConfig, ma_sound_group*pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_uninit(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_engine* ma_sound_group_get_engine(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_group_start(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_sound_group_stop(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_volume(ma_sound_group* pGroup, float volume);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_volume(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_pan(ma_sound_group* pGroup, float pan);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_pan(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_pan_mode(ma_sound_group* pGroup, ma_pan_mode panMode);
    [LibraryImport(LibraryName)]
    public static partial ma_pan_mode ma_sound_group_get_pan_mode(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_pitch(ma_sound_group* pGroup, float pitch);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_pitch(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_spatialization_enabled(ma_sound_group* pGroup, ma_bool32 enabled);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_group_is_spatialization_enabled(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_pinned_listener_index(ma_sound_group* pGroup, ma_uint32 listenerIndex);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_sound_group_get_pinned_listener_index(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_uint32 ma_sound_group_get_listener_index(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_group_get_direction_to_listener(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_position(ma_sound_group* pGroup, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_group_get_position(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_direction(ma_sound_group* pGroup, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_group_get_direction(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_velocity(ma_sound_group* pGroup, float x, float y, float z);
    [LibraryImport(LibraryName)]
    public static partial ma_vec3f ma_sound_group_get_velocity(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_attenuation_model(ma_sound_group* pGroup, ma_attenuation_model attenuationModel);
    [LibraryImport(LibraryName)]
    public static partial ma_attenuation_model ma_sound_group_get_attenuation_model(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_positioning(ma_sound_group* pGroup, ma_positioning positioning);
    [LibraryImport(LibraryName)]
    public static partial ma_positioning ma_sound_group_get_positioning(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_rolloff(ma_sound_group* pGroup, float rolloff);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_rolloff(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_min_gain(ma_sound_group* pGroup, float minGain);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_min_gain(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_max_gain(ma_sound_group* pGroup, float maxGain);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_max_gain(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_min_distance(ma_sound_group* pGroup, float minDistance);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_min_distance(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_max_distance(ma_sound_group* pGroup, float maxDistance);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_max_distance(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_cone(ma_sound_group* pGroup, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_get_cone(ma_sound_group* pGroup, float* pInnerAngleInRadians, float* pOuterAngleInRadians, float* pOuterGain);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_doppler_factor(ma_sound_group* pGroup, float dopplerFactor);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_doppler_factor(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_directional_attenuation_factor(ma_sound_group* pGroup, float directionalAttenuationFactor);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_directional_attenuation_factor(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_fade_in_pcm_frames(ma_sound_group* pGroup, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_fade_in_milliseconds(ma_sound_group* pGroup, float volumeBeg, float volumeEnd, ma_uint64 fadeLengthInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial float ma_sound_group_get_current_fade_volume(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_start_time_in_pcm_frames(ma_sound_group* pGroup, ma_uint64 absoluteGlobalTimeInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_start_time_in_milliseconds(ma_sound_group* pGroup, ma_uint64 absoluteGlobalTimeInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_stop_time_in_pcm_frames(ma_sound_group* pGroup, ma_uint64 absoluteGlobalTimeInFrames);
    [LibraryImport(LibraryName)]
    public static partial void ma_sound_group_set_stop_time_in_milliseconds(ma_sound_group* pGroup, ma_uint64 absoluteGlobalTimeInMilliseconds);
    [LibraryImport(LibraryName)]
    public static partial ma_bool32 ma_sound_group_is_playing(ma_sound_group* pGroup);
    [LibraryImport(LibraryName)]
    public static partial ma_uint64 ma_sound_group_get_time_in_pcm_frames(ma_sound_group* pGroup);
    #endregion


#if !MA_NO_RESOURCE_MANAGER
    public partial struct ma_resource_manager
    {
    }

    [LibraryImport(LibraryName)]
    public static partial ma_resource_manager* ma_engine_get_resource_manager(ma_engine pEngine);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_resource_manager_register_file(ma_resource_manager* pResourceManager, string pFilePath, ma_uint32 flags);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf16)]
    public static partial ma_result ma_resource_manager_register_file_w(ma_resource_manager* pResourceManager, string pFilePath, ma_uint32 flags);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_resource_manager_register_decoded_data(ma_resource_manager* pResourceManager, string pName, void* pData, ma_uint64 frameCount, ma_format format, ma_uint32 channels, ma_uint32 sampleRate);  /* Does not copy. Increments the reference count if already exists and returns MA_SUCCESS. */
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf16)]
    public static partial ma_result ma_resource_manager_register_decoded_data_w(ma_resource_manager* pResourceManager, string pName, void* pData, ma_uint64 frameCount, ma_format format, ma_uint32 channels, ma_uint32 sampleRate);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_resource_manager_register_encoded_data(ma_resource_manager* pResourceManager, string pName, void* pData, nuint sizeInBytes);    /* Does not copy. Increments the reference count if already exists and returns MA_SUCCESS. */
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf16)]
    public static partial ma_result ma_resource_manager_register_encoded_data_w(ma_resource_manager* pResourceManager, string pName, void* pData, nuint sizeInBytes);
    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_resource_manager_unregister_file(ma_resource_manager* pResourceManager, string pFilePath);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf16)]
    public static partial ma_result ma_resource_manager_unregister_data_w(ma_resource_manager* pResourceManager, string pName);
#endif


#if !MA_NO_DECODING
    public partial struct ma_decoder;

    [LibraryImport(LibraryName)]
    private static partial nuint ma_decoder_sizeof();

    public static ma_decoder* ma_ex_decoder_alloc()
    {
        return (ma_decoder*)ma_malloc(ma_decoder_sizeof());
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_uninit(ma_decoder* pDecoder);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_read_pcm_frames(ma_decoder* pDecoder, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_seek_to_pcm_frame(ma_decoder* pDecoder, ma_uint64 frameIndex);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_get_data_format(ma_decoder* pDecoder, out ma_format format, out ma_uint32 channels, out ma_uint32 sampleRate, ma_channel* pChannelMap, nuint channelMapCap);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_get_cursor_in_pcm_frames(ma_decoder* pDecoder, ma_uint64 pCursor);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_get_length_in_pcm_frames(ma_decoder* pDecoder, out ma_uint64 length);

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_decoder_get_available_frames(ma_decoder* pDecoder, out ma_uint64 availableFrames);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_decoder_init_memory(void* pData, nuint dataSize, /*const ma_decoder_config**/ nint pConfig, ma_decoder* pDecoder);

    [LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    public static partial ma_result ma_decoder_init_file(string pFilePath, /*const ma_decoder_config**/nint pConfig, ma_decoder* pDecoder);

    //[LibraryImport(LibraryName)]
    //public static partial ma_result ma_decode_from_vfs(ma_vfs* pVFS, const char* pFilePath, ma_decoder_config*pConfig, ma_uint64*pFrameCountOut, void** ppPCMFramesOut);

    //[LibraryImport(LibraryName, StringMarshalling = StringMarshalling.Utf8)]
    //public static partial ma_result ma_decode_file(string pFilePath, ma_decoder_config*pConfig, ma_uint64*pFrameCountOut, void** ppPCMFramesOut);

    //[LibraryImport(LibraryName)]
    //public static partial ma_result ma_decode_memory(void* pData, nuint dataSize, ma_decoder_config* pConfig, ma_uint64* pFrameCountOut, void** ppPCMFramesOut);
#endif

    /* Stereo panner. */
    public enum ma_pan_mode
    {
        ma_pan_mode_balance = 0,    /* Does not blend one side with the other. Technically just a balance. Compatible with other popular audio engines and therefore the default. */
        ma_pan_mode_pan             /* A true pan. The sound from one side will "move" to the other side and blend with it. */
    }

    public struct ma_panner_config
    {
        public ma_format format;
        public ma_uint32 channels;
        public ma_pan_mode mode;
        public float pan;
    }

    [LibraryImport(LibraryName)]
    public static partial ma_panner_config ma_panner_config_init(ma_format format, ma_uint32 channels);


    public struct ma_panner
    {
        public ma_format format;
        public ma_uint32 channels;
        public ma_pan_mode mode;
        public float pan;  /* -1..1 where 0 is no pan, -1 is left side, +1 is right side. Defaults to 0. */
    }

    [LibraryImport(LibraryName)]
    public static partial ma_result ma_panner_init(ma_panner_config* pConfig, ma_panner* pPanner);
    [LibraryImport(LibraryName)]
    public static partial ma_result ma_panner_process_pcm_frames(ma_panner* pPanner, void* pFramesOut, void* pFramesIn, ma_uint64 frameCount);
    [LibraryImport(LibraryName)]
    public static partial void ma_panner_set_mode(ma_panner* pPanner, ma_pan_mode mode);
    [LibraryImport(LibraryName)]
    public static partial ma_pan_mode ma_panner_get_mode(ma_panner* pPanner);
    [LibraryImport(LibraryName)]
    public static partial void ma_panner_set_pan(ma_panner* pPanner, float pan);
    [LibraryImport(LibraryName)]
    public static partial float ma_panner_get_pan(ma_panner* pPanner);

    #region UTF8OwnedMarshaler
    [CustomMarshaller(typeof(string), MarshalMode.ManagedToUnmanagedOut, typeof(UTF8OwnedMarshaler))]
    public static class UTF8OwnedMarshaler
    {
        /// <summary>
        /// Converts an unmanaged string to a managed version.
        /// </summary>
        /// <returns>A managed string.</returns>
        public static string? ConvertToManaged(byte* unmanaged)
        {
            if (unmanaged == null)
                return null;

            return UTF8EncodingRelaxed.Default.GetString(MemoryMarshal.CreateReadOnlySpanFromNullTerminated(unmanaged));
        }

        internal sealed class UTF8EncodingRelaxed : UTF8Encoding
        {
            public static new readonly UTF8EncodingRelaxed Default = new();

            private UTF8EncodingRelaxed() : base(false, false)
            {
            }
        }
    }
    #endregion
}
