// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Audio.Null;

internal class NullAudioDevice : AudioDevice
{
    public NullAudioDevice()
        : base(AudioBackendType.Null)
    {
        ApiName = "Null";
        ApiVersion = new Version(0, 0, 0);
    }

    /// <inheritdoc />
    public override string ApiName { get; }

    /// <inheritdoc />
    public override Version ApiVersion { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="NullAudioDevice" /> class.
    /// </summary>
    ~NullAudioDevice() => Dispose(isDisposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool isDisposing)
    {
        if (isDisposing)
        {
        }
    }

    /// <inheritdoc />
    protected override void OnMasterVolumeChanged(float volume)
    {

    }
}
