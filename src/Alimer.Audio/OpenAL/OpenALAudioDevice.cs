// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static OpenAL;

namespace Alimer.Audio.OpenAL;

internal unsafe sealed class OpenALAudioDevice : AudioDevice
{
    private readonly nint _device;

    public OpenALAudioDevice(in AudioDeviceOptions options)
        : base(in options)
    {
        _device = alcOpenDevice(null);
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="OpenALAudioDevice" /> class.
    /// </summary>
    ~OpenALAudioDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            alcCloseDevice(_device);
        }
    }

    /// <inheritdoc />
    protected override void SetMasterVolume(float value)
    {

    }
}
