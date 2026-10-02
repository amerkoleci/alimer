// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Defines the state of a window.
/// </summary>
public enum WindowState
{
    /// <summary>
    /// The window is restored.
    /// </summary>
    Normal = 0,
    /// <summary>
    /// The window is minimized.
    /// </summary>
    Minimized = 1,
    /// <summary>
    /// The window is maximized.
    /// </summary>
    Maximized = 2,
    /// <summary>
    /// The window is fullscreen.
    /// </summary>
    FullScreen,
    /// <summary>
    /// The window is in exclusive fullscreen mode.
    /// </summary>
    ExclusiveFullScreen,
}
