// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using Alimer.Assets;
using Alimer.Graphics;

#pragma warning disable CS0649

/* Callback for when a sound reaches the end. */
using unsafe stbi_write_func = delegate* unmanaged<nint /*context*/, void* /*data*/, int /*size*/, void>;

namespace Alimer;

internal unsafe static partial class AlimerApi
{
    public const string LibraryName = "alimer_native";

    #region STB
    [LibraryImport(LibraryName)]
    public static partial void stbi_free(void* ptr);

    [LibraryImport(LibraryName)]
    public static partial int stbi_is_16_bit_from_memory(ReadOnlySpan<byte> buffer, int len);

    [LibraryImport(LibraryName)]
    public static partial ushort* stbi_load_16_from_memory(ReadOnlySpan<byte> buffer, int len, int* x, int* y, int* channels_in_file, int desired_channels);

    [LibraryImport(LibraryName)]
    public static partial int stbi_is_hdr_from_memory(ReadOnlySpan<byte> buffer, int len);


    [LibraryImport(LibraryName)]
    public static partial float* stbi_loadf_from_memory(ReadOnlySpan<byte> buffer, int len, int* x, int* y, int* comp, int req_comp);

    [LibraryImport(LibraryName)]
    public static partial byte* stbi_load_from_memory(ReadOnlySpan<byte> buffer, int len, int* x, int* y, int* channels_in_file, int desired_channels);

    [LibraryImport(LibraryName)]
    public static partial void stbi_write_free(void* ptr);

    [LibraryImport(LibraryName)]
    public static partial int stbi_write_png_to_func(stbi_write_func func, nint context, int w, int h, int comp, void* data, int stride_in_bytes);

    [LibraryImport(LibraryName)]
    public static partial int stbi_write_bmp_to_func(stbi_write_func func, nint context, int w, int h, int comp, void* data);
    [LibraryImport(LibraryName)]
    public static partial int stbi_write_tga_to_func(stbi_write_func func, nint context, int w, int h, int comp, void* data);
    [LibraryImport(LibraryName)]
    public static partial int stbi_write_hdr_to_func(stbi_write_func func, nint context, int w, int h, int comp, float* data);
    [LibraryImport(LibraryName)]
    public static partial int stbi_write_jpg_to_func(stbi_write_func func, nint context, int x, int y, int comp, void* data, int quality);

    // tinyexr
    public const int TINYEXR_SUCCESS = 0;
    public const int TINYEXR_ERROR_INVALID_MAGIC_NUMBER = -1;
    public const int TINYEXR_ERROR_INVALID_EXR_VERSION = -2;
    public const int TINYEXR_ERROR_INVALID_ARGUMENT = -3;
    public const int TINYEXR_ERROR_INVALID_DATA = -4;
    public const int TINYEXR_ERROR_INVALID_FILE = -5;
    public const int TINYEXR_ERROR_INVALID_PARAMETER = -6;
    public const int TINYEXR_ERROR_CANT_OPEN_FILE = -7;
    public const int TINYEXR_ERROR_UNSUPPORTED_FORMAT = -8;
    public const int TINYEXR_ERROR_INVALID_HEADER = -9;
    public const int TINYEXR_ERROR_UNSUPPORTED_FEATURE = -10;
    public const int TINYEXR_ERROR_CANT_WRITE_FILE = -11;
    public const int TINYEXR_ERROR_SERIALIZATION_FAILED = -12;
    public const int TINYEXR_ERROR_LAYER_NOT_FOUND = -13;
    public const int TINYEXR_ERROR_DATA_TOO_LARGE = -14;

    [LibraryImport(LibraryName)]
    public static partial int alimerIsEXRFromMemory(ReadOnlySpan<byte> pData, nuint dataSize); // TINYEXR_SUCCESS

    [LibraryImport(LibraryName)]
    public static partial int alimerLoadEXRFromMemory(float** out_rgba, int* width, int* height, ReadOnlySpan<byte> memory, nuint size, byte** err);

    [LibraryImport(LibraryName)]
    public static partial void alimerFreeEXRErrorMessage(byte* msg);
    #endregion

    #region Font
    // private structure
    public struct stbtt__buf
    {
        public byte* data;
        public int cursor;
        public int size;
    }

    public struct stbtt_fontinfo
    {
        public void* userdata;
        public byte* data;              // pointer to .ttf file
        public int fontstart;         // offset of start of font

        public int numGlyphs;                     // number of glyphs, needed for range checking

        public int loca, head, glyf, hhea, hmtx, kern, gpos, svg; // table locations as offset from start of .ttf
        public int index_map;                     // a cmap mapping for our chosen character encoding
        public int indexToLocFormat;              // format needed to map from glyph index to glyph

        public stbtt__buf cff;                    // cff font data
        public stbtt__buf charstrings;            // the charstring index
        public stbtt__buf gsubrs;                 // global charstring subroutines index
        public stbtt__buf subrs;                  // private charstring subroutines index
        public stbtt__buf fontdicts;              // array of font dicts
        public stbtt__buf fdselect;               // map from glyph to fontdict
    }

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetNumberOfFonts(ReadOnlySpan<byte> data);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetFontOffsetForIndex(ReadOnlySpan<byte> data, int index);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_InitFont(stbtt_fontinfo* info, ReadOnlySpan<byte> data, int offset);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_FindGlyphIndex(stbtt_fontinfo* info, int unicode_codepoint);

    [LibraryImport(LibraryName)]
    public static partial float stbtt_ScaleForPixelHeight(stbtt_fontinfo* info, float pixels);
    [LibraryImport(LibraryName)]
    public static partial float stbtt_ScaleForMappingEmToPixels(stbtt_fontinfo* info, float pixels);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_GetFontVMetrics(stbtt_fontinfo* info, out int ascent, out int descent, out int lineGap);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetFontVMetricsOS2(stbtt_fontinfo* info, out int typoAscent, out int typoDescent, out int typoLineGap);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_GetFontBoundingBox(stbtt_fontinfo* info, int* x0, int* y0, int* x1, int* y1);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_GetCodepointHMetrics(stbtt_fontinfo* info, int codepoint, out int advanceWidth, out int leftSideBearing);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetCodepointKernAdvance(stbtt_fontinfo* info, int ch1, int ch2);
    // an additional amount to add to the 'advance' value between ch1 and ch2

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetCodepointBox(stbtt_fontinfo* info, int codepoint, int* x0, int* y0, int* x1, int* y1);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_GetGlyphHMetrics(stbtt_fontinfo* info, int glyph_index, int* advanceWidth, int* leftSideBearing);
    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetGlyphKernAdvance(stbtt_fontinfo* info, int glyph1, int glyph2);
    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetGlyphBox(stbtt_fontinfo* info, int glyph_index, int* x0, int* y0, int* x1, int* y1);

    [LibraryImport(LibraryName)]
    public static partial int stbtt_IsGlyphEmpty(stbtt_fontinfo* info, int glyph_index);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_MakeGlyphBitmapSubpixel(stbtt_fontinfo* info, byte* output, int out_w, int out_h, int out_stride, float scale_x, float scale_y, float shift_x, float shift_y, int glyph);

    [LibraryImport(LibraryName)]
    public static partial void stbtt_GetGlyphBitmapBox(stbtt_fontinfo* font, int glyph, float scale_x, float scale_y, int* ix0, int* iy0, int* ix1, int* iy1);

    public struct stbtt_kerningentry
    {
        public int glyph1; // use stbtt_FindGlyphIndex
        public int glyph2;
        public int advance;
    }

    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetKerningTableLength(stbtt_fontinfo* info);
    [LibraryImport(LibraryName)]
    public static partial int stbtt_GetKerningTable(stbtt_fontinfo* info, stbtt_kerningentry* table, int table_length);
    #endregion
}
