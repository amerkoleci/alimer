// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Toolkit.Diagnostics;
using Vortice.XAudio2;
using static Vortice.XAudio2.XAudio2;

namespace Vortice.Audio
{
    public sealed unsafe class XAudio2System : AudioSystem
    {
        private readonly IXAudio2 _xaudio2;

        public XAudio2System()
        {
            _xaudio2 = XAudio2Create();
        }
    }
}
