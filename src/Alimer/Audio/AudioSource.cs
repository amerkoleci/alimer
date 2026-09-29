// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Rendering;
using static Alimer.AlimerApi;
namespace Alimer.Audio;

public class AudioSource : DisposableObject
{
    internal AudioSource(AudioEngine engine, AudioClip clip, nint handle)
    {
        Engine = engine;
        Clip = clip;
        Handle = handle;
    }

    public AudioEngine Engine { get; }
    public AudioClip Clip { get; }
    public nint Handle { get; }
    public bool IsPlaying => alimerAudioSourceIsPlaying(Handle);

    /// <inheritdoc/>
    protected override void DisposeUnmanagedResources()
    {
        // Dispose unmanaged resources
        _ = alimerAudioSourceRelease(Handle);
    }

    public void Play()
    {
        alimerAudioSourcePlay(Handle);
    }

    public void Stop()
    {
        alimerAudioSourceStop(Handle);
    }
}
