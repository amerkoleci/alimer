// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.RHI;

/// <summary>
/// Specifies comparison functions used for depth/stencil tests or sampler tests
/// </summary>
public enum CompareFunction
{
    /// <summary>
    /// Comparison never passes.
    /// </summary>
    Never = 0,
    /// <summary>
    /// Passes if new value &lt; existing value.
    /// </summary>
    Less = 1,
    /// <summary>
    /// Passes if new value == existing value.
    /// </summary>
    Equal = 2,
    /// <summary>
    /// Passes if new value &lt;= existing value.
    /// </summary>
    LessEqual = 3,
    /// <summary>
    /// Passes if new value &gt; existing value.
    /// </summary>
    Greater = 4,
    /// <summary>
    /// Passes if new value != existing value.
    /// </summary>
    NotEqual = 5,
    /// <summary>
    /// Passes if new value &gt;= existing value.
    /// </summary>
    GreaterEqual = 6,
    /// <summary>
    /// Comparison always passes.
    /// </summary>
    Always = 7,
}
