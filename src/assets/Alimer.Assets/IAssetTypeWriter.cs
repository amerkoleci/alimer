// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public interface IAssetTypeWriter
{
    Type AssetType { get; }

    string FileExtension { get; }
    string MagicNumber { get; }
    ushort Version { get; }

    void Write(AssetWriter writer, Asset asset);
}
