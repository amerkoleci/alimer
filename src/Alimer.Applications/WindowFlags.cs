// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// <see cref="Window"/> Creation Flags
/// </summary>
[Flags]
public enum WindowFlags
{
    /// <summary>
    /// None,
    /// </summary>
    None = 0,

    Fullscreen = 0x1,
    Hidden = 0x2,
    Borderless = 0x4,
    Resizable = 0x8,
    Maximized = 0x10,
    AlwaysOnTop = 0x20,
}
