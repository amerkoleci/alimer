// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Numerics;
using static Alimer.AlimerApi;

namespace Alimer.Assets;

public sealed class Font : Asset
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

    private readonly nint _handle;

    private readonly Dictionary<int, int> _codepointToGlyphLookup = [];

    private Font(nint handle)
    {
        Debug.Assert(handle != null);

        _handle = handle;

        // Get font properties
        alimerFontGetMetrics(_handle, out int ascent, out int descent, out int linegap);
        Ascent = ascent;
        Descent = descent;
        LineGap = linegap;
    }

    /// <inheritdoc/>
    protected override void Destroy()
    {
        alimerFontDestroy(_handle);
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

    public static unsafe Font FromMemory(Span<byte> data)
    {
        fixed (byte* dataPtr = data)
        {
            nint handle = alimerFontCreateFromMemory(dataPtr, (uint)data.Length);
            if (handle == 0)
                throw new Exception("Unable to parse Font Data");

            return new Font(handle);
        }
    }

    /// <summary>
	/// Gets the Glyph Index of a given Unicode Codepoint
	/// </summary>
	public int GetGlyphIndex(int codepoint)
    {
        if (!_codepointToGlyphLookup.TryGetValue(codepoint, out int glyphIndex))
        {
            _codepointToGlyphLookup[codepoint] = glyphIndex = alimerFontGetGlyphIndex(_handle, codepoint);
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
	public float GetScale(float size)
    {
        Debug.Assert(_handle != 0, "Invalid Font");


        return alimerFontGetScale(_handle, size);
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
        Debug.Assert(_handle != 0, "Invalid Font");

        return alimerFontGetKerning(_handle, glyph1, glyph2, scale);
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
        alimerFontGetCharacter(_handle, glyphIndex, scale,
            out int width, out int height, out float advance, out float offsetX, out float offsetY, out int visible);

        return new()
        {
            GlyphIndex = glyphIndex,
            Width = width,
            Height = height,
            Advance = advance,
            Offset = new Vector2(offsetX, offsetY),
            Scale = scale,
            Visible = visible != 0,
        };
    }

    /// <summary>
	/// Renders a glyph to the given <see cref="Span{ColorRgba}" /> destination.
	/// </summary>
	public unsafe bool GetPixels(in Glyph glyph, Span<ColorRgba> destination)
    {
        if (!glyph.Visible)
            return false;

        if (destination.Length < glyph.Width * glyph.Height)
            return false;

        fixed (ColorRgba* ptr = destination)
        {
            alimerFontGetPixels(_handle, ptr, glyph.GlyphIndex, glyph.Width, glyph.Height, glyph.Scale);
        }

        return true;
    }
}
