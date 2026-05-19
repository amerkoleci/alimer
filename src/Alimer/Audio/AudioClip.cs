// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;
using static Alimer.AlimerApi;

namespace Alimer.Audio;

public sealed class AudioClip : Asset
{
    private AudioClip(nint handle)
    {
        if (handle == 0)
        {
            throw new InvalidOperationException();
        }

        Handle = handle;
    }

    internal nint Handle { get; }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        if (Handle != 0)
        {
            _ = alimerAudioClipRelease(Handle);
        }
    }

    public static AudioClip FromFile(string filePath)
    {
        nint handle = alimerAudioClipCreate(filePath);
        if (handle == 0)
        {
            throw new InvalidOperationException();
        }

        return new AudioClip(handle);
    }

    public static AudioClip FromStream(Stream stream)
    {
        Span<byte> data = stream.Length < 2048 ? stackalloc byte[(int)stream.Length] : new byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(data);
    }

    public static unsafe AudioClip FromMemory(ReadOnlySpan<byte> data)
    {
        fixed (byte* dataPtr = data)
        {
            nint handle = alimerAudioClipCreateFromMemory(dataPtr, (nuint)data.Length);
            if (handle == 0)
            {
                throw new InvalidOperationException();
            }

            return new AudioClip(handle);
        }
    }
}
