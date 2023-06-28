// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Describes the CPU access for <see cref="Buffer"/> and <see cref="Texture"/>.
/// </summary>
public enum CpuAccessMode
{
    None = 0,
    Write,
    Read,
}
