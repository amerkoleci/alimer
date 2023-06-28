// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Audio;

public abstract class AudioDevice : AudioObjectBase
{
    private float _masterVolume = 1.0f;

    public AudioDevice(AudioBackendType backendType)
    {
        BackendType = backendType;
    }

    /// <summary>
    /// Get the device backend type.
    /// </summary>
    public AudioBackendType BackendType { get; }

    /// <summary>
    /// Gets the underlying API Name.
    /// </summary>
    public abstract string ApiName { get; }

    /// <summary>
    /// Gets the underlying API version.
    /// </summary>
    public abstract Version ApiVersion { get; }

    public float MasterVolume
    {
        get => _masterVolume;
        set
        {
            _masterVolume = value;
            OnMasterVolumeChanged(value);
        }
    }

    public static bool IsBackendSupport(AudioBackendType backend)
    {
        Guard.IsTrue(backend != AudioBackendType.Count, nameof(backend), "Invalid backend");

        switch (backend)
        {
            case AudioBackendType.Null:
                return true;

#if !EXCLUDE_XAUDIO2_BACKEND
            case AudioBackendType.XAudio2:
                return XAudio2.XAudio2Device.IsSupported();
#endif

#if !EXCLUDE_OPENAL_BACKEND
            case AudioBackendType.OpenAL:
                return OpenAL.OpenALDevice.IsSupported();
#endif

            default:
                return false;
        }
    }

    public static AudioDevice CreateDefault(in AudioDeviceOptions options)
    {
        AudioBackendType backend = options.PreferredBackend;
        if (backend == AudioBackendType.Count)
        {
            if (IsBackendSupport(AudioBackendType.XAudio2))
            {
                backend = AudioBackendType.XAudio2;
            }
            else if (IsBackendSupport(AudioBackendType.OpenAL))
            {
                backend = AudioBackendType.OpenAL;
            }
        }

        AudioDevice? device = default;
        switch (backend)
        {
#if !EXCLUDE_XAUDIO2_BACKEND 
            case AudioBackendType.XAudio2:
                if (XAudio2.XAudio2Device.IsSupported())
                {
                    device = new XAudio2.XAudio2Device(in options);
                }
                break;
#endif

#if !EXCLUDE_OPENAL_BACKEND
            case AudioBackendType.OpenAL:
                if (OpenAL.OpenALDevice.IsSupported())
                {
                    device = new OpenAL.OpenALDevice(in options);
                }
                break;
#endif

            default:
            case AudioBackendType.Null:
                return new Null.NullAudioDevice();
        }

        if (device == null)
        {
            throw new AudioException($"{backend} is not supported");
        }

        return device!;
    }

    protected abstract void OnMasterVolumeChanged(float volume);
}
