// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base class for Asset.
/// </summary>
public abstract class Asset
{
    /// <summary>
    /// Gets or sets the name of the asset.
    /// </summary>
    public string? Name { get;set; }

    public Guid Id { get; set; } = Guid.NewGuid();

    public override string ToString()
    {
        return $"{GetType().Name}: {Id}";
    }
}
