// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using static Win32.Apis;
using static Win32.Media.Audio.XAudio2.Apis;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using Win32.Media.Audio.XAudio2;
using Win32;
using Win32.Media.Audio;

namespace Alimer.Audio.XAudio2;

internal unsafe class XAudio2Device : AudioDevice
{
    private static readonly Lazy<bool> s_isSupported = new(CheckIsSupported);

    private readonly AudioStreamCategory _category = AudioStreamCategory.GameEffects;

    private readonly ComPtr<IXAudio2> _xaudio2 = default;
    private XAudio2EngineCallback* _engineCallback;
    private IXAudio2MasteringVoice* _masterVoice;
    private IXAudio2SubmixVoice* _reverbVoice = default;

    private readonly uint _masterChannelMask;
    private readonly uint _masterChannels;
    private readonly uint _masterRate;
    private readonly X3DAudioHandle _X3DAudio;

    public static bool IsSupported() => s_isSupported.Value;

    public XAudio2Device(in AudioDeviceDescriptor descriptor)
        : base(AudioBackendType.XAudio2)
    {
        ApiName = "XAudio2";
        ApiVersion = new Version(2, 9, 0);

        HResult hr = XAudio2Create(_xaudio2.GetAddressOf());
        ThrowIfFailed(hr);

        if ((descriptor.Flags & AudioDeviceFlags.Debug) != 0)
        {
            DebugConfiguration debug = new()
            {
                TraceMask = LogType.Errors | LogType.Warnings,
                BreakMask = LogType.Errors
            };
            _xaudio2.Get()->SetDebugConfiguration(&debug);
            Debug.WriteLine("INFO: XAudio 2.9 debugging enabled");
        }

        XAudio2EngineCallback.Create(out _engineCallback);
        hr = _xaudio2.Get()->RegisterForCallbacks((IXAudio2EngineCallback*)_engineCallback);
        if (hr.Failure)
        {
            _xaudio2.Dispose();
            return;
        }

        IXAudio2MasteringVoice* masterVoice;
        hr = _xaudio2.Get()->CreateMasteringVoice(
            &masterVoice,
            XAUDIO2_DEFAULT_CHANNELS,
            XAUDIO2_DEFAULT_SAMPLERATE,
            0u,
            null,
            null,
            _category
            );
        if (hr.Failure)
        {
            _xaudio2.Dispose();
            return;
        }

        _masterVoice = masterVoice;

        uint dwChannelMask;
        hr = _masterVoice->GetChannelMask(&dwChannelMask);
        if (hr.Failure)
        {
            _masterVoice->DestroyVoice();
            _masterVoice = default;
            _xaudio2.Dispose();
            return;
        }

        VoiceDetails details;
        _masterVoice->GetVoiceDetails(&details);

        _masterChannelMask = dwChannelMask;
        _masterChannels = details.InputChannels;
        _masterRate = details.InputSampleRate;
        Debug.WriteLine($"Mastering voice has {_masterChannels} channels, {_masterRate} sample rate, {_masterChannelMask} channels");

        // Setup 3D audio
        hr = X3DAudioInitialize(_masterChannelMask, X3DAUDIO_SPEED_OF_SOUND, out _X3DAudio);
        if (hr.Failure)
        {
            if (_reverbVoice != null)
            {
                _reverbVoice->DestroyVoice();
                _reverbVoice = default;
            }

            _masterVoice->DestroyVoice();
            _masterVoice = default;
            //_reverbEffect.Reset();
            //_volumeLimiter.Reset();
            _xaudio2.Dispose();
        }
    }

    /// <inheritdoc />
    public override string ApiName { get; }

    /// <inheritdoc />
    public override Version ApiVersion { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="XAudio2Device" /> class.
    /// </summary>
    ~XAudio2Device() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            if (_reverbVoice != null)
            {
                _reverbVoice->DestroyVoice();
                _reverbVoice = default;
            }

            _masterVoice->DestroyVoice();
            _masterVoice = default;
            _xaudio2.Dispose();
            XAudio2EngineCallback.Free(_engineCallback);
        }
    }

    /// <inheritdoc />
    protected override void OnMasterVolumeChanged(float volume)
    {
        Debug.Assert(volume >= -XAUDIO2_MAX_VOLUME_LEVEL && volume <= XAUDIO2_MAX_VOLUME_LEVEL);

        if (_masterVoice != null)
        {
            ThrowIfFailed(_masterVoice->SetVolume(volume, 0));
        }
    }

    private static bool CheckIsSupported()
    {
#if WINDOWS_UWP
        return true;
#else
        return OperatingSystem.IsWindows();
#endif
    }
}
