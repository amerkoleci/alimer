// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

/// <summary>
/// Base interface for a <see cref="IDisposable"/> interface.
/// </summary>
public interface IDisposableObject : IDisposable
{
    /// <summary>
    /// Gets <c>true</c> if the object has been disposed; otherwise, <c>false</c>.
    /// </summary>
    bool IsDisposed { get; }
}
