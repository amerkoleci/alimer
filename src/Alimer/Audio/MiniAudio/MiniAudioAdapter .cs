// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;
using static MiniAudioNative;
using static MiniAudioNative.ma_device_type;

namespace Alimer.Audio.MiniAudio;

internal unsafe class MiniAudioAdapter : AudioAdapter
{
    internal MiniAudioAdapter(ma_device_type deviceType, ma_device_info* pInfo)
    {
        Name = UTF8OwnedMarshaler.ConvertToManaged(pInfo->name) ?? string.Empty;
        Type = deviceType == ma_device_type_playback ? AudioAdapterType.Playback : AudioAdapterType.Capture;
        IsDefault = pInfo->isDefault;
    }

    public override string Name { get; }

    public override AudioAdapterType Type { get; }

    public override bool IsDefault { get; }
}
