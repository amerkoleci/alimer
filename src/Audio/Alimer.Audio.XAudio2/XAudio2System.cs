// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.XAudio2;
using static Vortice.XAudio2.XAudio2;

namespace Alimer.Audio;

public class XAudio2System : AudioSystem
{
    private readonly IXAudio2 _xaudio2;

    public XAudio2System()
    {
        _xaudio2 = XAudio2Create();
    }
}
