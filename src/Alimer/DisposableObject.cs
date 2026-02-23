// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using CommunityToolkit.Diagnostics;

namespace Alimer;

/// <summary>
/// Base class for a <see cref="IDisposable"/> interface.
/// </summary>
public abstract class DisposableObject : IDisposableObject//, IReferencable
{
    //private volatile uint _refCount = 1;

    private DisposeCollector _collector;
    private volatile uint _isDisposed = 0;

    /// <summary>
    /// Initializes a new instance of the <see cref="DisposableObject" /> class.
    /// </summary>
    protected DisposableObject()
    {
        _collector = new DisposeCollector();
        _isDisposed = 0;
    }

    /// <inheritdoc />
    public bool IsDisposed => _isDisposed != 0;

    /// <inheritdoc/>
    public DisposeCollector Collector
    {
        get
        {
            _collector.EnsureValid();
            return _collector;
        }
    }

    /// <inheritdoc />
    public void Dispose()
    {
        if (Interlocked.Exchange(ref _isDisposed, 1) == 0)
        {
            _collector.Dispose();

            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    /// <inheritdoc cref="Dispose()" />
    /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
    protected virtual void Dispose(bool disposing)
    {

    }

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

    /// <inheritdoc cref="DisposeCollector.Add{T}(T)" />
    protected internal T ToDispose<T>(T objectToDispose)
        where T : notnull
    {
        Guard.IsNotNull(objectToDispose, nameof(objectToDispose));

        return _collector.Add(objectToDispose);
    }

    /// <inheritdoc cref="DisposeCollector.RemoveAndDispose{T}(ref T)" />
    protected internal void RemoveAndDispose<T>([MaybeNull] ref T objectToDispose)
        where T : notnull
    {
        _collector.RemoveAndDispose(ref objectToDispose);
    }
}
