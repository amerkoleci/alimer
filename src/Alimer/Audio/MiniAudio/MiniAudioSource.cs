// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static MiniAudioNative;
using static MiniAudioNative.ma_sound_flags;

namespace Alimer.Audio.MiniAudio;

internal unsafe class MiniAudioSource : AudioSource
{
    private readonly MiniAudioEngine _engine;

    internal MiniAudioSource(MiniAudioEngine engine)
    {
        _engine = engine;
        Handle = ma_ex_sound_alloc();
    }

    public ma_sound* Handle { get; private set; }

    /// <inheritdoc/>
    public override bool IsLooping
    {
        get => base.IsLooping;
        set
        {
            base.IsLooping = value;

            if (IsValid)
            {
                ma_sound_set_looping(Handle, value);
            }
        }
    }

    /// <inheritdoc/>
    public override float Volume
    {
        get => base.Volume;
        set
        {
            base.Volume = value;

            if (IsValid)
            {
                ma_sound_set_volume(Handle, value);
            }
        }
    }

    public override bool PitchingEnabled
    {
        get => base.PitchingEnabled;
        set
        {
            base.PitchingEnabled = value;
        }
    }

    public override bool SpatializationEnabled
    {
        get => base.SpatializationEnabled;
        set
        {
            base.SpatializationEnabled = value;

            if (IsValid)
            {
                ma_sound_set_spatialization_enabled(Handle, value);
            }
        }
    }

    public override bool IsAtEnd => IsValid ? ma_sound_at_end(Handle) : false;

    /// <inheritdoc/>
    protected override void DisposeUnmanagedResources()
    {
        if (Handle != null)
        {
            ma_sound_uninit(Handle);
            ma_free(Handle);
            Handle = null;
        }

        base.DisposeUnmanagedResources();
    }

    /// <inheritdoc/>
    protected override void OnClipChanged()
    {
        if (IsValid)
        {
            ma_sound_uninit(Handle);
        }

        // TODO: Handle MA_SOUND_FLAG_NO_PITCH and MA_SOUND_FLAG_NO_SPATIALIZATION
        if (Clip is not null)
        {
            ma_sound_flags flags = 0;
            if (Clip.Streamed)
                flags |= MA_SOUND_FLAG_STREAM;

            if (IsLooping)
                flags |= MA_SOUND_FLAG_LOOPING;

            if (!PitchingEnabled)
                flags |= MA_SOUND_FLAG_NO_PITCH;

            if (!SpatializationEnabled)
                flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;

            ma_sound_init_from_data_source(_engine.Handle, (ma_data_source*)Clip.Handle, (uint)flags, null, Handle).CheckResult(nameof(ma_sound_init_from_data_source));

            IsValid = true;
            //ma_sound_set_looping(Handle, IsLooping);
            ma_sound_set_volume(Handle, Volume);
        }
        else
        {
            IsValid = false;
        }
    }

    /// <inheritdoc/>
    public override void Play()
    {
        if (State == AudioSourceState.Playing)
            return;

        ma_sound_start(Handle);
        State = ma_sound_is_playing(Handle) ? AudioSourceState.Playing : AudioSourceState.Stopped;
    }

    /// <inheritdoc/>
    public override void Pause()
    {
        if (State == AudioSourceState.Paused)
            return;

        ma_sound_stop(Handle);
        State = AudioSourceState.Paused;
    }

    /// <inheritdoc/>
    public override void Stop()
    {
        if (State == AudioSourceState.Stopped)
            return;

        ma_sound_stop(Handle);
        ma_sound_seek_to_pcm_frame(Handle, 0);
        State = AudioSourceState.Stopped;
    }
}
