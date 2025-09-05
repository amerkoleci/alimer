// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Audio.OpenAL.ALApi;

namespace Alimer.Audio.OpenAL;

internal unsafe sealed class OpenALAudioDevice : AudioDevice
{
    private readonly ALCdevice _device;
    private readonly ALCcontext _context;

    public OpenALAudioDevice(in AudioDeviceOptions options)
        : base(in options)
    {
        if (alcIsExtensionPresent(ALCdevice.Null, "ALC_ENUMERATION_EXT"u8))
        {
            string devices = alcGetString(ALCdevice.Null, AlcGetString.DeviceSpecifier)!;
            string defaultDeviceName = alcGetString(ALCdevice.Null, AlcGetString.DefaultDeviceSpecifier)!;
        }

        _device = alcOpenDevice((string?)null);
        CheckAlcError();
        
        bool SupportsHrtf = alcIsExtensionPresent(_device, "ALC_SOFT_HRTF");
        int majorVersion = alcGetInteger(_device, AlcGetInteger.MajorVersion);
        int minorVersion = alcGetInteger(_device, AlcGetInteger.MinorVersion);
        int frequency = alcGetInteger(_device, AlcGetInteger.Frequency);
        int refresh = alcGetInteger(_device, AlcGetInteger.Refresh);
        int MonoSources = alcGetInteger(_device, AlcGetInteger.MonoSources);
        int StereoSources = alcGetInteger(_device, AlcGetInteger.StereoSources);

        string extensions = alcGetString(_device, AlcGetString.Extensions)!;

        //int[] arguments = [ALC_FREQUENCY, options.SampleRate];
        _context = alcCreateContext(_device, (int*)null/* argument*/);
        CheckAlcError();
        alcMakeContextCurrent(_context);
        CheckAlcError();
        string vendor = alGetString(AlGetString.Vendor);
        string version = alGetString(AlGetString.Version);
        string renderer = alGetString(AlGetString.Renderer);
        string contextExtensions = alGetString(AlGetString.Extensions);
        alDistanceModel(AlDistanceModel.InverseDistanceClamped);
        CheckAlcError();
        bool SupportsIma4 = alIsExtensionPresent("AL_EXT_IMA4"u8);
        bool SupportsAdpcm = alIsExtensionPresent("AL_SOFT_MSADPCM"u8);
        bool SupportsEfx = alIsExtensionPresent("AL_EXT_EFX"u8);
        bool SupportsIeee = alIsExtensionPresent("AL_EXT_float32"u8);
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

    [System.Diagnostics.Conditional("DEBUG")]
    //[System.Diagnostics.DebuggerHidden]
    private void CheckAlcError()
    {
        AlcError error = alcGetError(_device);
        if (error != AlcError.NoError)
        {
            string formatErrMsg = $"OpenAL Error: {alcGetString(_device, (int)error)} - {alcGetCurrentContext()}";
            throw new AudioException(formatErrMsg);
        }
    }
}
