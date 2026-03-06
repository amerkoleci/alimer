// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

/// <summary>
/// Base class for a <see cref="IDisposable"/> interface.
/// </summary>
public abstract class DisposableObject : IDisposableObject
{
    private volatile uint _isDisposed;
    private DisposeCollector _collector;

    /// <summary>
    /// Initializes a new instance of the <see cref="DisposableObject" /> class.
    /// </summary>
    protected DisposableObject()
    {
        _collector = new DisposeCollector();
    }

    ~DisposableObject()
    {
        Dispose(disposing: false);
    }

    #region IDisposable Members + DisposeCollector
    /// <inheritdoc />
    public bool IsDisposed => _isDisposed is not 0;

    /// <summary>
    /// Gets the <see cref="DisposeCollector"/>
    /// </summary>
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
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    /// <inheritdoc cref="Dispose()" />
    /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
    protected virtual void Dispose(bool disposing)
    {
        if (disposing)
        {
            _collector.Dispose();
        }
    }

    /// <inheritdoc cref="DisposeCollector.Add{T}(T)" />
    protected internal T ToDispose<T>(T objectToDispose)
        where T : notnull
    {
        ArgumentNullException.ThrowIfNull(objectToDispose, nameof(objectToDispose));

        return _collector.Add(objectToDispose);
    }

    /// <inheritdoc cref="DisposeCollector.RemoveAndDispose{T}(ref T)" />
    protected internal void RemoveAndDispose<T>([MaybeNull] ref T objectToDispose)
        where T : notnull
    {
        _collector.RemoveAndDispose(ref objectToDispose);
    }
    #endregion
}
