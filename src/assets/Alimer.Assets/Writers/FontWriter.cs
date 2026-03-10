// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets.Graphics;
using Alimer.Graphics;
using Alimer.Serialization;

namespace Alimer.Assets.Writers;

internal class FontWriter : AssetTypeWriter<FontAsset>
{
    public override string FileExtension => "afnt";
    public override string MagicNumber => "AFNT";

    public override void Write(ref WriteByteStream writer, FontAsset asset)
    {
        // Write glyph information
        writer.Write(asset.Glyphs.Count);
        foreach (FontGlyph glyph in asset.Glyphs)
        {
            writer.Write(glyph.Character);
            writer.Write(glyph.Subrect);
            writer.Write(glyph.Offset);
            writer.Write(glyph.XAdvance);
        }

        // Write kerning information
        //writer.Write(font.KerningPairs.Count);
        //foreach (var kerning in font.KerningPairs)
        //{
        //    writer.Write(kerning.First);
        //    writer.Write(kerning.Second);
        //    writer.Write(kerning.Amount);
        //}

        // Write texture font data
        writer.Write(asset.AtlasWidth);
        writer.Write(asset.AtlasHeight);
        writer.Write(asset.AtlasData.Length);
        writer.Write(asset.AtlasData);
    }
}
