// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Serialization;

namespace Alimer.Assets;

public abstract class AssetTypeWriter<TAsset> : IAssetTypeWriter
    where TAsset : Asset
{
    public Type AssetType => typeof(TAsset);

    public abstract string FileExtension { get; }
    public abstract string MagicNumber { get; }

    public virtual ushort Version => 1;

    public abstract void Write(ref WriteByteStream writer, TAsset asset);

    void IAssetTypeWriter.Write(ref WriteByteStream writer, Asset asset)
    {
        if (asset is not TAsset typedAsset)
        {
            throw new ArgumentException($"Invalid asset");
        }

        // Write magic number and version
        writer.Write(MagicNumber);
        writer.Write(Version); // Version 1

        Write(ref writer, typedAsset);
    }
}
