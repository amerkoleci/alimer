// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static Alimer.AlimerApi;

namespace Alimer.Audio;

public sealed class AudioAdapter
{
    internal unsafe AudioAdapter(AlimerApi.AudioDevice device, AudioDeviceType type)
    {
        Name = Utf8CustomMarshaller.ConvertToManaged(alimerAudioDeviceGetName(device)) ?? string.Empty;
        Type = type;
        IsDefault = alimerAudioDeviceIsDefault(device);
    }

    public string Name { get; }
    public AudioDeviceType Type { get; }
    public bool IsDefault { get; }
}
