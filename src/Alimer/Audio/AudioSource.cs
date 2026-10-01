// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

public abstract class AudioSource : DisposableObject
{
    public bool IsValid { get; protected set; }
    public AudioSourceState State { get; protected set; } = AudioSourceState.Stopped;
    public virtual bool IsLooping { get; set; } = false;
    public virtual float Volume { get; set; } = 1.0f;
    public virtual bool PitchingEnabled { get; set; } = false;
    public virtual bool SpatializationEnabled { get; set; } = true;
    public abstract bool IsAtEnd { get; }

    public AudioClip? Clip
    {
        get;
        set
        {
            if (field != value)
            {
                field?.Release();
                field = value;
                OnClipChanged();
                value?.AddReference();
            }
        }
    }

    /// <inheritdoc/>
    protected override void DisposeUnmanagedResources()
    {
        Clip?.Release();
    }

    protected virtual void OnClipChanged()
    {
    }

    /// <summary>
    /// Plays the audio source.
    /// </summary>
    public abstract void Play();

    /// <summary>
    /// Pauses the audio source.
    /// </summary>
    public abstract void Pause();

    /// <summary>
    /// Stops the audio source.
    /// </summary>
    public abstract void Stop();
}
