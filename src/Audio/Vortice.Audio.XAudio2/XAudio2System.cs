// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Toolkit.Diagnostics;
using TerraFX.Interop;
using static TerraFX.Interop.Windows;
using static Vortice.Audio.XAudio2Helpers;

namespace Vortice.Audio
{
    public sealed unsafe class XAudio2System : AudioSystem
    {
        private readonly ComPtr<IXAudio2> _xaudio2;

        public XAudio2System()
        {
            ThrowIfFailed(XAudio2Create(_xaudio2.GetAddressOf()));
        }
    }
}
