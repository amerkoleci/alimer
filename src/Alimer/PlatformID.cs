// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Identifiers the running platform type.
/// </summary>
public enum PlatformID 
{
    /// <summary>
    /// Windows platform.
    /// </summary>
    Windows,
    /// <summary>
    /// Linux platform.
    /// </summary>
    Linux,
    /// <summary>
    /// macOS/MacCatalyst platform.
    /// </summary>
    MacOS,
    /// Android platform.
    Android,
    /// iOS platform.
    iOS,
    /// Browser (WASM) platform.
    Browser,
}
