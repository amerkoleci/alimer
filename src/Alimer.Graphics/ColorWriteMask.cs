// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

[Flags]
public enum ColorWriteMask : byte
{
    None = 0,
    Red = 0x01,
    Green = 0x02,
    Blue = 0x04,
    Alpha = 0x08,
    All = 0x0F
}
