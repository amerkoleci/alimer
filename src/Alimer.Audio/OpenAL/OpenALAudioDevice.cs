// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Bindings.OpenAL;
using static Alimer.Bindings.OpenAL.OpenAL;

namespace Alimer.Audio.OpenAL;

internal unsafe sealed class OpenALAudioDevice : AudioDevice
{
    private readonly ALCdevice _device;
    private readonly ALCcontext _context;

    public OpenALAudioDevice(in AudioDeviceOptions options)
        : base(in options)
    {
        if (alcIsExtensionPresent("ALC_enumeration_EXT"))
        {
            string name;
            if (!alcIsExtensionPresent("ALC_enumerate_all_EXT") )
                name = alcGetString(AlcGetString.DeviceSpecifier);
            else
                name = alcGetString(AlcGetString.AllDevicesSpecifier);

            name = alcGetString(AlcGetString.CaptureDeviceSpecifier);
        }

        _device = alcOpenDevice(null);
        CheckAlcError();
        
        bool SupportsHrtf = alcIsExtensionPresent(_device, "ALC_SOFT_HRTF");
        int majorVersion = alcGetInteger(_device, AlcGetInteger.MajorVersion);
        int minorVersion = alcGetInteger(_device, AlcGetInteger.MinorVersion);
        int frequency = alcGetInteger(_device, AlcGetInteger.Frequency);
        int refresh = alcGetInteger(_device, AlcGetInteger.Refresh);
        int MonoSources = alcGetInteger(_device, AlcGetInteger.MonoSources);
        int StereoSources = alcGetInteger(_device, AlcGetInteger.StereoSources);

        string extensions = alcGetString(_device, AlcGetString.Extensions);

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
        bool SupportsIma4 = alIsExtensionPresent("AL_EXT_IMA4");
        bool SupportsAdpcm = alIsExtensionPresent("AL_SOFT_MSADPCM");
        bool SupportsEfx = alIsExtensionPresent("AL_EXT_EFX");
        bool SupportsIeee = alIsExtensionPresent("AL_EXT_float32");
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
            string formatErrMsg = string.Format("OpenALc Error: {0} - {1}", alcGetString(_device, error), alcGetCurrentContext().ToString());
            throw new AudioException(formatErrMsg);
        }
    }
}
