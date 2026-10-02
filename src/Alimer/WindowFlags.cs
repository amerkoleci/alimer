// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Creation flags for a window.
/// </summary>
[Flags]
public enum WindowFlags
{
    /// <summary>
    /// None,
    /// </summary>
    None = 0,
    Fullscreen = 1 << 0,
    ExclusiveFullscreen = 1 << 1,
    Hidden = 1 << 2,
    Borderless = 1 << 3,
    Resizable = 1 << 4,
    Maximized = 1 << 5,
    AlwaysOnTop = 1 << 6,
}
