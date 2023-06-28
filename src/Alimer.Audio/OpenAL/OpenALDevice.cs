// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;
using static Alimer.Audio.OpenAL.OpenALNative;

namespace Alimer.Audio.OpenAL;

internal unsafe class OpenALDevice : AudioDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);
    public static bool IsSupported() => s_isSupported.Value;

    private readonly ALCdevice _device;
    private readonly ALCcontext _context;

    public OpenALDevice(in AudioDeviceOptions descriptor)
        : base(AudioBackendType.OpenAL)
    {
        ApiName = "OpenAL";

        _device = alcOpenDevice(null);
        int* attributes = stackalloc int[]
        {
            ALC_FREQUENCY,
            descriptor.SampleRate
        };
        _context = alcCreateContext(_device, attributes);
        Guard.IsTrue(alcMakeContextCurrent(_context) == 1);
        Debug.Assert(_context == alcGetCurrentContext());

        int alcMajorVersion, alcMinorVersion;
        alcGetIntegerv(_device, ALC_MAJOR_VERSION, 1, &alcMajorVersion);
        alcGetIntegerv(_device, ALC_MINOR_VERSION, 1, &alcMinorVersion);
        string alcExts = new(alcGetString(_device, ALC_EXTENSIONS));

        ApiVersion = new Version(alcMajorVersion, alcMinorVersion, 0);
    }

    /// <inheritdoc />
    public override string ApiName { get; }

    /// <inheritdoc />
    public override Version ApiVersion { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="OpenALDevice" /> class.
    /// </summary>
    ~OpenALDevice() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            alcCloseDevice(_device);
        }
    }

    /// <inheritdoc />
    protected override void OnMasterVolumeChanged(float volume)
    {
        //Debug.Assert(volume >= -XAUDIO2_MAX_VOLUME_LEVEL && volume <= XAUDIO2_MAX_VOLUME_LEVEL);
    }

    private static bool CheckIsSupported()
    {
        return true;
    }
}
