// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;
using static Alimer.AlimerApi;

namespace Alimer.Graphics;

public sealed unsafe class Font : DisposableObject
{
    private readonly nint _handle;

    private readonly Dictionary<int, int> _codepointToGlyphLookup = new();

    private Font(nint handle)
    {
        Guard.IsTrue(handle != 0);

        _handle = handle;

        // Get font properties
        Alimer_FontGetMetrics(_handle, out int ascent, out int descent, out int linegap);
        Ascent = ascent;
        Descent = descent;
        LineGap = linegap;
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            Alimer_FontDestroy(_handle);
        }
    }

    public int Ascent { get; }
    public int Descent { get; }
    public int LineGap { get; }
    public int Height => Ascent - Descent;
    public int LineHeight => Ascent - Descent + LineGap; // Leading

    public static Font FromFile(string filePath, bool srgb = true)
    {
        using FileStream stream = new(filePath, FileMode.Open);
        return FromStream(stream, srgb);
    }

    public static Font FromStream(Stream stream, bool srgb = true)
    {
        byte[] data = new byte[stream.Length];
        stream.ReadExactly(data, 0, (int)stream.Length);
        return FromMemory(data, srgb);
    }

    public static unsafe Font FromMemory(byte[] data, bool srgb = true)
    {
        fixed (byte* dataPtr = data)
        {
            nint handle = Alimer_FontCreateFromMemory(dataPtr, (uint)data.Length);
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
            _codepointToGlyphLookup[codepoint] = glyphIndex = Alimer_FontGetGlyphIndex(_handle, codepoint);
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

        return Alimer_FontGetScale(_handle, size);
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

        return Alimer_FontGetKerning(_handle, glyph1, glyph2, scale);
    }
}
