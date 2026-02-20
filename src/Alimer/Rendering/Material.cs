// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Assets;

namespace Alimer.Rendering;

public enum MaterialAlphaMode
{
    Opaque,
    Mask,
    Blend
}

/// <summary>
/// Base material class.
/// </summary>
public abstract class Material : Asset, IDisposableObject
{
    private volatile uint _isDisposed = 0;

    /// <inheritdoc />
    public bool IsDisposed => _isDisposed != 0;
    public MaterialAlphaMode AlphaMode { get;set; } = MaterialAlphaMode.Opaque;
    public bool Transparent => AlphaMode == MaterialAlphaMode.Blend;

    ~Material()
    {
        Dispose(false);
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
        Destroy();
    }
}
