// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Numerics;
using static Alimer.AlimerApi;

namespace Alimer.Assets;

public sealed unsafe class Font : Asset
{
    public struct Glyph
    {
        public int GlyphIndex;
        public int Width;
        public int Height;
        public float Advance;
        public Vector2 Offset;
        public float Scale;
        public bool Visible;
    }

    private readonly stbtt_fontinfo* _fontInfo;

    private readonly Dictionary<int, int> _codepointToGlyphLookup = [];

    private Font(stbtt_fontinfo* fontInfo)
    {
        _fontInfo = fontInfo;

        // Get font properties
        stbtt_GetFontVMetrics(_fontInfo, out int ascent, out int descent, out int linegap);
        Ascent = ascent;
        Descent = descent;
        LineGap = linegap;

        int advance, bearing;
        stbtt_GetCodepointHMetrics(_fontInfo, ' ', out advance, out bearing);
        // font->spaceAdvance = advance;
    }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        NativeMemory.Free(_fontInfo);
    }

    public int Ascent { get; }
    public int Descent { get; }
    public int LineGap { get; }
    public int Height => Ascent - Descent;
    public int LineHeight => Ascent - Descent + LineGap; // Leading

    public static Font FromFile(string filePath)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(stream);
    }

    public static Font FromStream(Stream stream)
    {
        Span<byte> data = stream.Length < 2048 ? stackalloc byte[(int)stream.Length] : new byte[(int)stream.Length];
        stream.ReadExactly(data);
        return FromMemory(data);
    }

    public static Font FromMemory(ReadOnlySpan<byte> data)
    {
        int offset = stbtt_GetFontOffsetForIndex(data, 0);
        if (offset == -1)
        {
            throw new Exception("Unable to parse Font Data");
        }

        stbtt_fontinfo* fontInfo = (stbtt_fontinfo*)NativeMemory.AllocZeroed((nuint)sizeof(stbtt_fontinfo));
        if (stbtt_InitFont(fontInfo, data, offset) == 0)
        {
            throw new Exception("Unable to parse Font Data");
        }

        return new Font(fontInfo);
    }

    /// <summary>
	/// Gets the Glyph Index of a given Unicode Codepoint
	/// </summary>
	public int GetGlyphIndex(int codepoint)
    {
        if (!_codepointToGlyphLookup.TryGetValue(codepoint, out int glyphIndex))
        {
            _codepointToGlyphLookup[codepoint] = glyphIndex = stbtt_FindGlyphIndex(_fontInfo, codepoint);
        }

        return glyphIndex;
    }

    /// <summary>
	/// Gets the Glyph Index of the given Char
	/// </summary>
	public int GetGlyphIndex(char ch)
    {
        return GetGlyphIndex((int)ch);
    }

    /// <summary>
	/// Gets the scale value of the Font for a requested size in pixels
	/// </summary>
	public float GetScale(float pixels)
    {
        return stbtt_ScaleForMappingEmToPixels(_fontInfo, pixels);
    }

    public float GetScalePixelHeight(float pixelHeight)
    {
        return stbtt_ScaleForPixelHeight(_fontInfo, pixelHeight);
    }

    /// <summary>
	/// Gets the kerning value between two chars at a given scale
	/// </summary>
	public float GetKerning(char ch1, char ch2, float scale)
    {
        int glyph1 = GetGlyphIndex(ch1);
        int glyph2 = GetGlyphIndex(ch2);
        return GetKerningBetweenGlyphs(glyph1, glyph2, scale);
    }

    /// <summary>
    /// Gets the kerning value between two unicode codepoints at a given scale
    /// </summary>
    public float GetKerning(int codepoint1, int codepoint2, float scale)
    {
        int glyph1 = GetGlyphIndex(codepoint1);
        int glyph2 = GetGlyphIndex(codepoint2);
        return GetKerningBetweenGlyphs(glyph1, glyph2, scale);
    }

    /// <summary>
    /// Gets the kerning value between two glyphs at a given scale
    /// </summary>
    public float GetKerningBetweenGlyphs(int glyph1, int glyph2, float scale)
    {
        return stbtt_GetGlyphKernAdvance(_fontInfo, glyph1, glyph2) * scale;
    }

    /// <summary>
	/// Gets Glyph Metrics of a given char at a given scale
	/// </summary>
	public Glyph GetGlyph(char character, float scale)
    {
        return GetGlyphByIndex(GetGlyphIndex(character), scale);
    }

    /// <summary>
    /// Gets Glyph Metrics of a given unicode codepoint at a given scale
    /// </summary>
    public Glyph GetGlyph(int codepoint, float scale)
    {
        return GetGlyphByIndex(GetGlyphIndex(codepoint), scale);
    }

    /// <summary>
	/// Gets Glyph Metrics of a given glyph at a given scale
	/// </summary>
	public Glyph GetGlyphByIndex(int glyphIndex, float scale)
    {
        int adv, ox, x0, y0, x1, y1;

        stbtt_GetGlyphHMetrics(_fontInfo, glyphIndex, &adv, &ox);
        stbtt_GetGlyphBitmapBox(_fontInfo, glyphIndex, scale, scale, &x0, &y0, &x1, &y1);

        int width = (x1 - x0);
        int height = (y1 - y0);

        return new()
        {
            GlyphIndex = glyphIndex,
            Width = width,
            Height = height,
            Advance = adv * scale,
            Offset = new Vector2(ox * scale, (float)y0),
            Scale = scale,
            Visible = (width > 0 && height > 0 && stbtt_IsGlyphEmpty(_fontInfo, glyphIndex) == 0),
        };
    }

    /// <summary>
	/// Renders a glyph to the given <see cref="Span{ColorRgba}" /> destination.
	/// </summary>
	public bool GetPixels(in Glyph glyph, Span<ColorRgba> destination)
    {
        if (!glyph.Visible)
            return false;

        if (destination.Length < glyph.Width * glyph.Height)
            return false;

        fixed (ColorRgba* ptr = destination)
        {
            // parse it directly into the dest buffer
            GetPixels(_fontInfo, (byte*)ptr, glyph.GlyphIndex, glyph.Width, glyph.Height, glyph.Scale, true);
        }

        return true;
    }

    private static void GetPixels(stbtt_fontinfo* info, byte* dst, int glyph, int width, int height, float scale, bool premultiply = true)
    {
        // parse it directly into the dest buffer
        stbtt_MakeGlyphBitmapSubpixel(info, dst, width, height, width, scale, scale, 0, 0, glyph);

        // convert the buffer to RGBA data by working backwards, overwriting data
        int len = width * height;
        for (int a = (len - 1) * 4, b = (len - 1); b >= 0; a -= 4, b -= 1)
        {
            dst[a + 0] = (byte)(premultiply ? dst[b] : 255);
            dst[a + 1] = (byte)(premultiply ? dst[b] : 255);
            dst[a + 2] = (byte)(premultiply ? dst[b] : 255);
            dst[a + 3] = dst[b];
        }
    }

}
