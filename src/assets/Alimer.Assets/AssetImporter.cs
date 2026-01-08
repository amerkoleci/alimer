// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public abstract class AssetImporter<T, TMetadata> : IAssetImporter
    where T : Asset
    where TMetadata : AssetMetadata
{
    public Type AssetMetadataType => typeof(TMetadata);

    public abstract Task<T> Import(TMetadata metadata);

    Task<Asset> IAssetImporter.Import(AssetMetadata metadata)
    {
        if (metadata is not TMetadata typedMetadata)
        {
            throw new ArgumentException($"Invalid metadata type for {GetType()}, needs {typeof(TMetadata)}");
        }

        return Import(typedMetadata).ContinueWith(item => (Asset)item.Result);
    }
}
