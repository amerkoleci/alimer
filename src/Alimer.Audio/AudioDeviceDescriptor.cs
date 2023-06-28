// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio;

/// <summary>
/// Structure that describes the <see cref="AudioDevice"/>.
/// </summary>
public record struct AudioDeviceDescriptor
{
    public AudioDeviceDescriptor()
    {
    }

    /// <summary>
    /// Gets or sets the preferred backend to creates.
    /// </summary>
    public AudioBackendType PreferredBackend { get; set; } = AudioBackendType.Count;

    /// <summary>
    /// Gets the <see cref="AudioDevice"/> creation flags.
    /// </summary>
    public AudioDeviceFlags Flags { get; set; } = AudioDeviceFlags.Default;

    /// <summary>
    /// The sample rate to which all sources will be resampled to (if required). 
    /// Then coverts to the maximum supported sample rate by the device.
    /// </summary>
    public int SampleRate { get; set; } = 44100;

    /// <summary>
    /// The number of channels that this device is sammpling
    /// </summary>
    public int SampleChannels { get; set; } = 2;

    // <summary>
    /// Gets or sets the label of <see cref="GraphicsDevice"/>.
    /// </summary>
    public string? Label { get; set; } = default;
}
