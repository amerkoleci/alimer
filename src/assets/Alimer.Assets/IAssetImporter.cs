// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public interface IAssetImporter
{
    Type AssetMetadataType { get; }

    Task<Asset> Import(AssetMetadata  metadata);
}
