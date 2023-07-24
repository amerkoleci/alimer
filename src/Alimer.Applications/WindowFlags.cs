// Copyright © Amer Koleci and Contributors.
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

    Borderless = (1 << 0),
    Resizable = (1 << 1),
    Maximized = (1 << 2),
    Fullscreen = (1 << 3),
}
