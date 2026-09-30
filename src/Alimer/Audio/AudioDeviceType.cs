// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Specifies the type of an <see cref="AudioDevice"/>.
/// </summary>
public enum AudioDeviceType
{
    /// <summary>
    /// Specifies a playback audio adapter.
    /// </summary>
    Playback,
    /// <summary>
    /// Specifies a capture audio adapter.
    /// </summary>
    Capture,
}
