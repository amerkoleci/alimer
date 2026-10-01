// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Alimer.Assets;
using static MiniAudioNative;
using static MiniAudioNative.ma_format;

namespace Alimer.Audio;

public sealed unsafe class AudioClip : Asset
{
    private AudioClip(ma_decoder* handle)
    {
        if (handle == null)
        {
            throw new InvalidOperationException();
        }

        Handle = handle;

        ma_decoder_get_data_format(Handle, out ma_format format, out uint channels, out uint sampleRate, null, 0).CheckResult(nameof(ma_decoder_get_data_format));
        ma_decoder_get_length_in_pcm_frames(Handle, out ulong frames).CheckResult(nameof(ma_decoder_get_length_in_pcm_frames));
        ma_decoder_get_available_frames(Handle, out ulong availableFrames).CheckResult(nameof(ma_decoder_get_available_frames));

        Format = FromMiniaudio(format);
        ChannelCount = channels;
        SampleRate = sampleRate;
        Stride = channels * Format.GetSampleSize();
        Streamed = true;
    }

    internal ma_decoder* Handle { get; }
    public AudioFormat Format { get; set; }
    public uint ChannelCount { get; set; }
    public uint SampleRate { get; set; }
    public ulong FrameCount { get; set; }
    public uint Stride { get; set; }
    public bool Streamed { get; set; }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        if (Handle != null)
        {
            ma_decoder_uninit(Handle).CheckResult(nameof(ma_decoder_uninit));
            ma_free(Handle);
        }
    }

    public static AudioClip FromFile(string filePath)
    {
        ma_decoder* decoder = ma_ex_decoder_alloc();
        ma_result result = ma_decoder_init_file(filePath, 0, decoder);
        if (result != ma_result.MA_SUCCESS)
        {
            throw new AudioException();
        }

        return new AudioClip(decoder);
    }

    //public static AudioClip FromStream(Stream stream)
    //{
    //    Span<byte> data = stream.Length < 2048 ? stackalloc byte[(int)stream.Length] : new byte[(int)stream.Length];
    //    stream.ReadExactly(data);
    //    return FromMemory(data);
    //}

    //public static unsafe AudioClip FromMemory(ReadOnlySpan<byte> data)
    //{
    //    fixed (byte* dataPtr = data)
    //    {
    //        nint handle = alimerAudioClipCreateFromMemory(dataPtr, (nuint)data.Length);
    //        if (handle == 0)
    //        {
    //            throw new InvalidOperationException();
    //        }
    //
    //        return new AudioClip(handle);
    //    }
    //}

    private static bool IsWAV(ReadOnlySpan<byte> data)
    {
        return data.Length >= 12 &&
               data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F' &&
               data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E';
    }

    private static bool IsOGG(ReadOnlySpan<byte> data)
    {
        return data.Length >= 4 && data[0] == 'O' && data[1] == 'g' && data[2] == 'g' && data[3] == 'S';
    }

    private static bool IsFLAC(ReadOnlySpan<byte> data)
    {
        return data.Length >= 4 && data[0] == 'f' && data[1] == 'L' && data[2] == 'a' && data[3] == 'C';
    }

    private static bool IsMP3(ReadOnlySpan<byte> data)
    {
        return data.Length >= 3 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static AudioFormat FromMiniaudio(ma_format value)
    {
        return value switch
        {
            ma_format_unknown => AudioFormat.Unknown,
            ma_format_u8 => AudioFormat.Unsigned8,
            ma_format_s16 => AudioFormat.Signed16,
            ma_format_s24 => AudioFormat.Signed24,
            ma_format_s32 => AudioFormat.Signed32,
            ma_format_f32 => AudioFormat.Float32,
            _ => AudioFormat.Unknown,
        };
    }
}
