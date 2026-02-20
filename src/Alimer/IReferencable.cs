// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Base interface for all objects that use reference-counting lifetime management.
/// </summary>
public interface IReferencable
{
    /// <summary>
    /// Gets the reference count of this instance.
    /// </summary>
    /// <value>The reference count.</value>
    uint ReferenceCount { get; }

    /// <summary>
    /// Increments the reference count of this instance.
    /// </summary>
    /// <returns>The new reference count.</returns>
    uint AddReference();

    /// <summary>
    /// Decrements the reference count of this instance.
    /// </summary>
    /// <returns>The new reference count.</returns>
    /// <remarks>
    /// When the reference count reaches 0, the component should release / dispose dependent objects.
    /// </remarks>
    uint Release();
}
