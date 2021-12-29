// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Vortice.Audio.OpenAL.OpenALNative;

namespace Alimer.Audio;

public unsafe class OpenALSystem : AudioSystem
{
    private readonly nint _device;

    public OpenALSystem()
    {
#if NET5_0_OR_GREATER
        _device = alcOpenDevice(null);
#endif
    }
}
