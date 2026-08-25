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
    None = 0x0000,
    Fullscreen = 0x0001,
    Hidden = 0x0002,
    Borderless = 0x0004,
    Resizable = 0x0008,
    Maximized = 0x0010,
    AlwaysOnTop = 0x0020,
}
