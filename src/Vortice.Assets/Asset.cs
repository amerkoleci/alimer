// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Assets;

/// <summary>
/// Base class for Asset.
/// </summary>
public abstract class Asset
{
    public Guid Id { get; set; } = Guid.NewGuid();

    public override string ToString()
    {
        return $"{GetType().Name}: {Id}";
    }
}
