// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base class for Asset.
/// </summary>
public abstract class Asset : IAsset
{
    private volatile uint _refCount =1;

    /// <inheritdoc />
    public Guid Id { get; set; } = Guid.NewGuid();

    /// <inheritdoc />
    public string Name { get; set; } = string.Empty;

    /// <inheritdoc/>
    public uint ReferenceCount { get { return _refCount; } }

    protected Asset()
    {
    }

    /// <inheritdoc cref="IReferencable.AddReference()" />
    public virtual uint AddReference()
    {
        uint newRefCount = Interlocked.Increment(ref _refCount);
        if (newRefCount <= 1)
            throw new InvalidOperationException("Cannot add a reference for an object already released. AddReference/Release pair must match.");
        return newRefCount;
    }

    /// <inheritdoc cref="IReferencable.Release()" />
    public virtual uint Release()
    {
        uint newRefCount = Interlocked.Decrement(ref _refCount);
        if (newRefCount == 0)
        {
            try
            {
                Destroy();
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

    public override string ToString()
    {
        return $"{GetType().Name}: {Id}";
    }

    /// <summary>
    /// Releases unmanaged and - optionally - managed resources
    /// </summary>
    protected abstract void Destroy();
}
