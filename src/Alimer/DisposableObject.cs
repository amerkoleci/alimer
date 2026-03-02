// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

/// <summary>
/// Base class for a <see cref="IDisposable"/> interface.
/// </summary>
public abstract class DisposableObject : IDisposableObject, IReferencable
{
    private uint _refCount = 1;
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
        Dispose();
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

    /// <summary>
    /// Decrements the reference count of this object, disposing, releasing, and freeing associated resources when the count reaches zero.
    /// </summary>
    /// <seealso cref="Destroy"/>
    public void Dispose()
    {
        if (Interlocked.Exchange(ref _isDisposed, 1) is not 0)
            return;

        _ = Release();
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Disposes the object's resources.
    /// </summary>
    /// <remarks>
    ///   <para>
    ///     Override in a derived class to implement disposal logic specific to it.
    ///   </para>
    ///   <para>
    ///     This method is automatically called whenever a call to <see cref="Dispose"/> (or to <see cref="IReferencable.Release"/>)
    ///     has decreased the internal reference count to zero, meaning no other objects (hopefully) hold a reference to this one
    ///     and its resources can be safely released.
    ///   </para>
    /// </remarks>
    protected virtual void Destroy()
    {
        _collector.Dispose();
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

    #region IReferencable Members 
    /// <inheritdoc/>
    public uint ReferenceCount => _refCount;

    /// <inheritdoc cref="IReferencable.AddReference()" />
    public uint AddReference()
    {
        OnAddReference();

        uint newRefCount = Interlocked.Increment(ref _refCount);
        if (newRefCount <= 1)
            throw new InvalidOperationException("Cannot add a reference for an object already released. AddReference/Release pair must match.");
        return newRefCount;
    }

    /// <inheritdoc cref="IReferencable.Release()" />
    public uint Release()
    {
        OnReleaseReference();

        uint newRefCount = Interlocked.Decrement(ref _refCount);
        if (newRefCount == 0)
        {
            try
            {
                Destroy();
                _ = Interlocked.Exchange(ref _isDisposed, 1);
            }
            finally
            {
                // Reverse back the counter if there are any exceptions in the destroy method
                Interlocked.Exchange(ref _refCount, newRefCount + 1);
            }
        }
        else if (newRefCount < 0)
        {
            throw new InvalidOperationException("Cannot release an object that doesn't have active reference. AddReference/Release pair must match.");
        }

        return newRefCount;
    }

    /// <summary>
    /// Called when a new reference of this object has been counted (via a call to <see cref="IReferencable.AddReference"/>).
    /// </summary>
    protected virtual void OnAddReference()
    {
    }

    /// <summary>
    /// Called when a call to <see cref="IReferencable.Release"/> has decremented the reference count of this object.
    /// </summary>
    protected virtual void OnReleaseReference()
    {
    }
    #endregion
}
