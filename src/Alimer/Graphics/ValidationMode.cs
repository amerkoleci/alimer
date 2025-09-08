// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum ValidationMode
{
    /// <summary>
    /// No validation is enabled.
    /// </summary>
    Disabled,
    /// <summary>
    /// Print warnings and errors.
    /// </summary>
    Enabled,
    /// <summary>
    /// Print all warnings, errors and info messages.
    /// </summary>
    Verbose,
    /// Enable GPU-based validation
    GPU
}
