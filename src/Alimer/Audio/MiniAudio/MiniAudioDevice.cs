// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static MiniAudioNative;

namespace Alimer.Audio.MiniAudio;

internal unsafe class MiniAudioDevice : AudioDevice
{
    private readonly ma_device_info* _info;

    internal MiniAudioDevice(AudioDeviceType deviceType, ma_device_info* info)
    {
        _info = info;

        Name = UTF8OwnedMarshaler.ConvertToManaged(_info->name) ?? string.Empty;
        Type = deviceType;
        IsDefault = _info->isDefault;
    }

    /// <inheritdoc />
    public override string Name { get; }

    /// <inheritdoc />
    public override AudioDeviceType Type { get; }

    /// <inheritdoc />
    public override bool IsDefault { get; }
}
