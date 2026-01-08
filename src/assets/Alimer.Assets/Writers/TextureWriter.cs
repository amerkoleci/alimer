// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets.Graphics;
using Alimer.Graphics;

namespace Alimer.Assets.Writers;

internal class TextureWriter : AssetTypeWriter<TextureAsset>
{
    public override string FileExtension => "atex";
    public override string MagicNumber => "TEXB";

    public override void Write(AssetWriter writer, TextureAsset asset)
    {
        // Write dimensions
        Debug.Assert((int)PixelFormat.Count <= byte.MaxValue);

        writer.Write((byte)asset.Dimension);
        writer.Write((byte)asset.Format);
        writer.Write(asset.Width);
        writer.Write(asset.Height);
        writer.Write(asset.DepthOrArrayLayers);
        writer.Write(asset.MipLevels);

        // Write texture data
        writer.Write(asset.Data!.Length);
        writer.Write(asset.Data);
    }
}
