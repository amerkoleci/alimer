// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base intefface for Asset.
/// </summary>
public interface IAsset : IReferencable
{
    /// <summary>
    /// Gets the unique identifier for this asset.
    /// </summary>
	Guid Id { get; }

    /// <summary>
    /// Gets the asset name.
    /// </summary>
    string Name { get; }
}
