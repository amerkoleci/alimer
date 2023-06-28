// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Operation to perform on the stencil value.
/// </summary>
public enum StencilOperation
{
    /// <summary>
    /// Keep stencil value unchanged.
    /// </summary>
    Keep = 0,
    /// <summary>
    /// Set stencil value to zero.
    /// </summary>
    Zero = 1,
    /// <summary>
    /// /// Replace stencil value with value provided in most recent call to SetStencilReference
    /// </summary>
    Replace = 2,
    /// <summary>
    /// /// Increments stencil value by one, clamping on overflow.
    /// </summary>
    IncrementClamp = 3,
    /// <summary>
    /// /// Decrements stencil value by one, clamping on underflow.
    /// </summary>
    DecrementClamp = 4,
    /// <summary>
    /// Bitwise inverts stencil value.
    /// </summary>
    Invert = 5,
    /// <summary>
    /// Increments stencil value by one, wrapping on overflow.
    /// </summary>
    IncrementWrap = 6,
    /// <summary>
    /// Decrements stencil value by one, wrapping on underflow.
    /// </summary>
    DecrementWrap = 7,
}
