// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

/// <summary>
/// Represents a device that provides pointer input, such as a mouse, pen, or touch screen.
/// </summary>
/// <remarks>Implementations of this interface enable unified handling of pointer-based input across different
/// device types. This allows applications to process pointer events in a device-agnostic manner.</remarks>
public abstract class PointerDevice : InputDevice
{
    /// <summary>
    /// Gets or sets the current cursor type.
    /// </summary>
    public abstract Cursor Cursor { get; set; }

    public virtual void Update()
    {
    }
}
