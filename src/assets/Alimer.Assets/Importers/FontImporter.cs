// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Assets.Graphics;
using FreeTypeSharp;
using SkiaSharp;
using SkiaSharp.HarfBuzz;
using static FreeTypeSharp.FT;
using static FreeTypeSharp.FT_Error;
using static FreeTypeSharp.FT_Render_Mode_;

namespace Alimer.Assets;

/// <summary>
/// Defines a <see cref="FontAsset"/> importer.
/// </summary>
[AssetImporter(".ttf")]
public sealed unsafe class FontImporter : AssetImporter<FontAsset, FontMetadata>
{
    private readonly FT_LibraryRec_* _lib;
    private const int FontDpi = 96;

    public bool UseFreeType { get; set; }

    public FontImporter()
    {
        FT_LibraryRec_* lib;
        FT_Error error = FT_Init_FreeType(&lib);
        if (error != FT_Err_Ok)
            throw new InvalidOperationException("Could not initialize FreeType library");

        _lib = lib;
    }

    public override Task<FontAsset> Import(FontMetadata metadata)
    {
        if (UseFreeType)
        {
            using Stream stream = File.Open(metadata.FileFullPath, FileMode.Open, FileAccess.Read);
            Span<byte> data = stackalloc byte[(int)stream.Length];
            stream.ReadExactly(data);
            FT_FaceRec_* face;

            fixed (byte* dataPtr = data)
            {
                FT_Error error = FT_New_Memory_Face(_lib, dataPtr, (nint)data.Length, 0, &face);
                error = FT_Set_Char_Size(face, 0, 16 * 64, FontDpi, FontDpi);
                uint glyphIndex = FT.FT_Get_Char_Index(face, 'я');
            }

            throw new NotImplementedException();
            //FontAsset asset = new()
            //{
            //    Source = metadata.FileFullPath
            //};
            //
            //return Task.FromResult(asset);
        }

        return ImportSkia(metadata);
    }

    private Task<FontAsset> ImportSkia(FontMetadata metadata)
    {
        string fontFamily = metadata.FontFamily ?? Path.GetFileNameWithoutExtension(metadata.FileFullPath);

        using (SKTypeface typeface = string.IsNullOrEmpty(Path.GetExtension(fontFamily)) ? SKTypeface.FromFamilyName(fontFamily) : SKTypeface.FromFile(fontFamily))
        {
            using SKShaper shaper = new(typeface);
            using (SKFont font = new(typeface, metadata.FontSize))
            {
                font.GetFontMetrics(out SKFontMetrics metrics);
                float lineHeight = metrics.Descent - metrics.Ascent;

                // Detect atlas size first
                List<SKSize> glyphSizes = [];

                foreach (char c in metadata.Characters)
                {
                    ushort glyphId = typeface.GetGlyph(c);
                    float widthGlyph = font.MeasureText(new string(c, 1), out SKRect bounds);
                    glyphSizes.Add(new SKSize(widthGlyph, lineHeight));
                }


                // Pack glyphs into a texture atlas
                List<SKRect> packedRects = PackRectangles(glyphSizes, out int atlasWidth, out int atlasHeight);

                // Create the texture atlas
                List<FontGlyph> glyphs = [];
                using (SKBitmap atlasBitmap = new(atlasWidth, atlasHeight, SKColorType.Bgra8888, SKAlphaType.Unpremul))
                using (SKCanvas atlasCanvas = new(atlasBitmap))
                {
                    atlasCanvas.Clear(SKColors.Transparent);

                    using (SKPaint paint = new())
                    {
                        paint.IsAntialias = true;

                        for (int i = 0; i < metadata.Characters.Length; i++)
                        {
                            char c = metadata.Characters[i];
                            SKRect packedRect = packedRects[i];

                            // Draw gradient fill
                            bool doGradientPaint = true;
                            if (doGradientPaint)
                            {
                                using (var gradientPaint = new SKPaint())
                                {
                                    gradientPaint.IsAntialias = true;
                                    var gradient = SKShader.CreateLinearGradient(
                                        new SKPoint(packedRect.Left, packedRect.Top),
                                        new SKPoint(packedRect.Left, packedRect.Top + packedRect.Height),
                                        new[] { SKColors.Green, SKColors.Blue },
                                        null,
                                        SKShaderTileMode.Clamp
                                    );
                                    gradientPaint.Shader = gradient;
                                    atlasCanvas.DrawText(c.ToString(), packedRect.Left, packedRect.Bottom, font, gradientPaint);
                                }
                            }

                            //atlasCanvas.DrawText(c.ToString(), packedRect.Left, packedRect.Bottom, font, paint);

                            if (c == 'B')
                            {
                            }

                            SKShaper.Result result = shaper.Shape(new string(c, 1), 0, 0, font);

                            glyphs.Add(new FontGlyph
                            {
                                Character = c,
                                Subrect = new Alimer.Numerics.RectI((int)packedRect.Left, (int)packedRect.Top, (int)packedRect.Width, (int)packedRect.Height),
                                //Offset = new(glyphBounds[i].Left, glyphBounds[i].Top),
                                XAdvance = (int)font.MeasureText(new string(c, 1))
                            });
                        }
                    }

#if TODO
                    // Generate kerning information
                    for (int i = 0; i < metadata.Characters.Length; i++)
                    {
                        for (int j = 0; j < metadata.Characters.Length; j++)
                        {
                            char first = metadata.Characters[i];
                            char second = metadata.Characters[j];
                            float kerning = typeface.GetKerning(typeface.GetGlyph(first), typeface.GetGlyph(second)) * metadata.FontSize / typeface.UnitsPerEm;
                            if (Math.Abs(kerning) > 0.01f)
                            {
                                font.KerningPairs.Add(new KerningPair { First = first, Second = second, Amount = (int)kerning });
                            }
                        }
                    } 
#endif

                    Span<SKColor> pixels = SkiaUtils.SKColorToColor(atlasBitmap.Pixels);

                    FontAsset asset = new()
                    {
                        AtlasWidth = atlasWidth,
                        AtlasHeight = atlasHeight,
                        AtlasData = MemoryMarshal.Cast<SKColor, byte>(pixels).ToArray(),
                        Glyphs = glyphs,
                        Source = metadata.FileFullPath
                    };

                    using var data = atlasBitmap.Encode(SKEncodedImageFormat.Png, 100);
                    using var stream = File.OpenWrite("output.png");
                    data.SaveTo(stream);

                    return Task.FromResult(asset);
                }
            }
        }
    }

    private static List<SKRect> PackRectangles(List<SKSize> sizes, out int atlasWidth, out int atlasHeight)
    {
        // Implement a rectangle packing algorithm here. For simplicity, we'll use a basic approach.
        int totalArea = (int)sizes.Sum(r => r.Width * r.Height);
        int estimatedSize = (int)Math.Ceiling(Math.Sqrt(totalArea) * 1.1); // Add 10% padding

        atlasWidth = atlasHeight = 1;
        while (atlasWidth * atlasHeight < estimatedSize * estimatedSize)
        {
            atlasWidth *= 2;
            atlasHeight *= 2;
        }

        List<SKRect> packedRects = [];
        int currentX = 0;
        int currentY = 0;
        int rowHeight = 0;

        foreach (SKSize size in sizes)
        {
            if (currentX + size.Width > atlasWidth)
            {
                currentX = 0;
                currentY += rowHeight;
                rowHeight = 0;
            }

            packedRects.Add(new SKRect(currentX, currentY, currentX + size.Width, currentY + size.Height));
            currentX += (int)size.Width;
            rowHeight = Math.Max(rowHeight, (int)size.Height);
        }

        return packedRects;
    }
}
