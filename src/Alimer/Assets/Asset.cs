// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base class for Asset.
/// </summary>
public abstract class Asset : DisposableObject, IAsset
{
    private volatile uint _refCount = 1;

    /// <inheritdoc />
    public Guid Id { get; set; } = Guid.NewGuid();

    /// <inheritdoc />
    public string Name { get; set; } = string.Empty;

    protected Asset()
    {
    }

    public override string ToString()
    {
        return $"{GetType().Name}: {Id}";
    }
}
