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

    /// <summary>
    /// Initializes a new instance of the <see cref="DisposableObject" /> class.
    /// </summary>
    protected DisposableObject()
    {
    }

    ~DisposableObject()
    {
        Dispose(disposing: false);
    }

    /// <inheritdoc />
    public bool IsDisposed => _isDisposed is not 0;

    /// <inheritdoc />
    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Throws an ObjectDisposedException if this object has been disposed
    /// </summary>
    protected void ThrowIfDisposed()
    {
        ObjectDisposedException.ThrowIf(IsDisposed, this);
    }

    private void Dispose(bool disposing)
    {
        if (Interlocked.CompareExchange(ref _isDisposed, 1, 0) is not 0)
            return;

        if (disposing)
        {
            DisposeManagedResources();
        }

        DisposeUnmanagedResources();
    }

    /// <summary>
    /// Releases the unmanaged resources used by the <see cref="DisposableObject"/> class.
    /// </summary>
    protected virtual void DisposeUnmanagedResources()
    {
    }

    /// <summary>
    /// Releases the managed resources used by the <see cref="DisposableObject"/> class.
    /// </summary>
    protected virtual void DisposeManagedResources()
    {
    }
}

public abstract class DisposableObjectWithCollector : DisposableObject
{
    private readonly DisposeCollector _collector = new();

    /// <summary>
    /// Initializes a new instance of the <see cref="DisposableObjectWithCollector" /> class.
    /// </summary>
    protected DisposableObjectWithCollector()
    {
    }

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

    /// <inheritdoc/>
    protected override void DisposeManagedResources()
    {
        base.DisposeManagedResources();

        _collector.Dispose();
    }

    /// <inheritdoc cref="DisposeCollector.Add{T}(T)" />
    protected internal T ToDispose<T>(T objectToDispose)
        where T : notnull
    {
        ThrowIfDisposed();
        ArgumentNullException.ThrowIfNull(objectToDispose, nameof(objectToDispose));

        return _collector.Add(objectToDispose);
    }

    /// <inheritdoc cref="DisposeCollector.RemoveAndDispose{T}(ref T)" />
    protected internal void RemoveAndDispose<T>([MaybeNull] ref T objectToDispose)
        where T : notnull
    {
        ThrowIfDisposed();
        _collector.RemoveAndDispose(ref objectToDispose);
    }
}
