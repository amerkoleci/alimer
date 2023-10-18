// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;

namespace Alimer;

/// <summary>
/// Base class for a <see cref="IDisposable"/> interface.
/// </summary>
public abstract class DisposableObject : IDisposable
{
    private volatile uint _isDisposed = 0;

    /// <summary>
    /// Initializes a new instance of the <see cref="DisposableObject" /> class.
    /// </summary>
    protected DisposableObject()
    {
        _isDisposed = 0;
    }

    /// <summary>
    /// Gets <c>true</c> if the object has been disposed; otherwise, <c>false</c>.
    /// </summary>
    public bool IsDisposed => _isDisposed != 0;

    /// <inheritdoc />
    public void Dispose()
    {
        if (Interlocked.Exchange(ref _isDisposed, 1) == 0)
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    /// <inheritdoc cref="Dispose()" />
    /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
    protected abstract void Dispose(bool disposing);

    /// <summary>Throws an exception if the object has been disposed.</summary>
    /// <exception cref="ObjectDisposedException">The object has been disposed.</exception>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    protected void ThrowIfDisposed()
    {
        if (_isDisposed != 0)
        {
            throw new ObjectDisposedException(ToString());
        }
    }
}
