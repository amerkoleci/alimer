// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

public enum AudioFormat
{
    Unknown,
    Unsigned8 = 1,
    Signed16 = 2,
    Signed24 = 3,
    Signed32 = 4,
    Float32
}

public static class AudioFormatExtensions
{
    public static int GetSampleSize(this AudioFormat format)
    {
        return format switch
        {
            AudioFormat.Unknown => 0,
            AudioFormat.Unsigned8 => 1,
            AudioFormat.Signed16 => 2,
            AudioFormat.Signed24 => 3,
            AudioFormat.Signed32 => 4,
            AudioFormat.Float32 => 4,
            _ => 0
        };
    }
}
