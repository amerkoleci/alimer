// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct VertexAttributeFormatInfo(
    VertexAttributeFormat Format,
    int ByteSize,
    int ComponentCount,
    FormatKind Kind
);

public static class VertexAttributeFormatUtils
{
    private static readonly VertexAttributeFormatInfo[] s_vertexFormatInfos =
   [
        new(VertexAttributeFormat.UByte,                    1, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UByte2,                   2, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UByte4,                   4, 4,   FormatKind.Uint),
        new(VertexAttributeFormat.Byte,                     1, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.Byte2,                    2, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.Byte4,                    4, 4,   FormatKind.Sint),
        new(VertexAttributeFormat.UByteNormalized,          1, 1,   FormatKind.Unorm),
        new(VertexAttributeFormat.UByte2Normalized,         2, 2,   FormatKind.Unorm),
        new(VertexAttributeFormat.UByte4Normalized,         4, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.ByteNormalized,           1, 1,   FormatKind.Snorm),
        new(VertexAttributeFormat.Byte2Normalized,          2, 2,   FormatKind.Snorm),
        new(VertexAttributeFormat.Byte4Normalized,          4, 4,   FormatKind.Snorm),

        new(VertexAttributeFormat.UShort,                   2, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UShort2,                  4, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UShort4,                  8, 4,   FormatKind.Uint),
        new(VertexAttributeFormat.Short,                    2, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.Short2,                   4, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.Short4,                   8, 4,   FormatKind.Sint),
        new(VertexAttributeFormat.UShortNormalized,         2, 1,   FormatKind.Unorm),
        new(VertexAttributeFormat.UShort2Normalized,        4, 2,   FormatKind.Unorm),
        new(VertexAttributeFormat.UShort4Normalized,        8, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.ShortNormalized,          2, 1,   FormatKind.Snorm),
        new(VertexAttributeFormat.Short2Normalized,         4, 2,   FormatKind.Snorm),
        new(VertexAttributeFormat.Short4Normalized,         8, 4,   FormatKind.Snorm),
        new(VertexAttributeFormat.Half,                     2, 1,   FormatKind.Float),
        new(VertexAttributeFormat.Half2,                    4, 2,   FormatKind.Float),
        new(VertexAttributeFormat.Half4,                    8, 4,   FormatKind.Float),
        new(VertexAttributeFormat.Float,                    4, 1,   FormatKind.Float),
        new(VertexAttributeFormat.Float2,                   8, 2,   FormatKind.Float),
        new(VertexAttributeFormat.Float3,                   12, 3,  FormatKind.Float),
        new(VertexAttributeFormat.Float4,                   16, 4,  FormatKind.Float),
        new(VertexAttributeFormat.UInt,                     4, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt2,                    8, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt3,                    12, 3,  FormatKind.Uint),
        new(VertexAttributeFormat.UInt4,                    16, 4,  FormatKind.Uint),
        new(VertexAttributeFormat.Int,                      4, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.Int2,                     8, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.Int3,                    12, 3,  FormatKind.Sint),
        new(VertexAttributeFormat.Int4,                    16, 4,  FormatKind.Sint),

        //new(VertexFormat.Int1010102Normalized,    32, 4, FormatKind.Unorm),
        new(VertexAttributeFormat.Unorm10_10_10_2,           4, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.Unorm8x4BGRA,              4, 4,   FormatKind.Unorm),
        //new(VertexFormat.RG11B10Float,              32, 4, FormatKind.Float),
        //new(VertexFormat.RGB9E5Float,               32, 4, FormatKind.Float),
   ];

    public static ref readonly VertexAttributeFormatInfo GetFormatInfo(this VertexAttributeFormat format)
    {
        if (format >= VertexAttributeFormat.Count)
        {
            return ref s_vertexFormatInfos[0]; // Undefines
        }

        Debug.Assert(s_vertexFormatInfos[(uint)format].Format == format);
        return ref s_vertexFormatInfos[(uint)format];
    }

    public static int GetSizeInBytes(this VertexAttributeFormat format) => GetFormatInfo(format).ByteSize;
}
