// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base class for runtime <see cref="IAsset"/> readers.
/// </summary>
public abstract class AssetReader(Type targetType) : IAssetReader
{
    /// <inheritdoc/>
    public Type TargetType { get; } = targetType;
}

public abstract class AssetReader<T> : AssetReader
    where T : class/*IAsset*/
{
    protected AssetReader()
        : base(typeof(T))
    {
    }
}
