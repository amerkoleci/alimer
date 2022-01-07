// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.XAUDIO2;

namespace Vortice.Audio;

public unsafe class XAudio2System : AudioSystem
{
    private readonly ComPtr<IXAudio2> _xaudio2 = default;

    public XAudio2System()
    {
         XAudio2Create(_xaudio2.GetAddressOf(), 0u, XAUDIO2_DEFAULT_PROCESSOR);
    }
}
