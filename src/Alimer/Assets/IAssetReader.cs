// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

/// <summary>
/// Base interface for runtime <see cref="IAsset"/> readers.
/// </summary>
public interface IAssetReader
{
    /// <summary>
    /// Gets the runtime type that the reader can load.
    /// </summary>
    Type TargetType { get; }
}
