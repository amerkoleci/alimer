// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Utilities;

namespace Alimer.Audio;

public sealed class AudioAdapter
{
    internal unsafe AudioAdapter(AlimerApi.AudioDevice* device, AudioDeviceType type)
    {
        Name = Utf8CustomMarshaller.ConvertToManaged(device->name) ?? string.Empty;
        Type = device->deviceType;
        IsDefault = device->isDefault;
    }

    public string Name { get; }
    public AudioDeviceType Type { get; }
    public bool IsDefault { get; }
}
