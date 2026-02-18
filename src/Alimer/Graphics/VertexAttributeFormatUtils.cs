// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

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
        new(VertexAttributeFormat.UInt8,                    1, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt8x2,                  2, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt8x4,                  4, 4,   FormatKind.Uint),
        new(VertexAttributeFormat.SInt8,                    1, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt8x2,                  2, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt8x4,                  4, 4,   FormatKind.Sint),
        new(VertexAttributeFormat.UNorm8,                   1, 1,   FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm8x2,                 2, 2,   FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm8x4,                 4, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.SNorm8,                   1, 1,   FormatKind.Snorm),
        new(VertexAttributeFormat.SNorm8x2,                 2, 2,   FormatKind.Snorm),
        new(VertexAttributeFormat.SNorm8x4,                 4, 4,   FormatKind.Snorm),

        new(VertexAttributeFormat.UInt16,                   2, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt16x2,                 4, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt16x4,                 8, 4,   FormatKind.Uint),
        new(VertexAttributeFormat.SInt8,                    2, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt8x2,                  4, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt8x4,                  8, 4,   FormatKind.Sint),
        new(VertexAttributeFormat.UNorm16,                  2, 1,   FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm16x2,                4, 2,   FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm16x4,                8, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.SNorm16,                  2, 1,   FormatKind.Snorm),
        new(VertexAttributeFormat.SNorm16x2,                4, 2,   FormatKind.Snorm),
        new(VertexAttributeFormat.SNorm16x2,                8, 4,   FormatKind.Snorm),
        new(VertexAttributeFormat.Float16,                  2, 1,   FormatKind.Float),
        new(VertexAttributeFormat.Float16x2,                4, 2,   FormatKind.Float),
        new(VertexAttributeFormat.Float16x4,                8, 4,   FormatKind.Float),
        new(VertexAttributeFormat.Float32,                  4, 1,   FormatKind.Float),
        new(VertexAttributeFormat.Float32x2,                8, 2,   FormatKind.Float),
        new(VertexAttributeFormat.Float32x3,                12, 3,  FormatKind.Float),
        new(VertexAttributeFormat.Float32x4,                16, 4,  FormatKind.Float),
        new(VertexAttributeFormat.UInt32,                   4, 1,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt32x2,                 8, 2,   FormatKind.Uint),
        new(VertexAttributeFormat.UInt32x3,                 12, 3,  FormatKind.Uint),
        new(VertexAttributeFormat.UInt32x4,                 16, 4,  FormatKind.Uint),
        new(VertexAttributeFormat.SInt32,                   4, 1,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt32x2,                 8, 2,   FormatKind.Sint),
        new(VertexAttributeFormat.SInt32x3,                 12, 3,  FormatKind.Sint),
        new(VertexAttributeFormat.SInt32x4,                 16, 4,  FormatKind.Sint),

        //new(VertexFormat.Int1010102Normalized,    32, 4, FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm10_10_10_2,           4, 4,   FormatKind.Unorm),
        new(VertexAttributeFormat.UNorm8x4BGRA,              4, 4,   FormatKind.Unorm),
        //new(VertexFormat.RG11B10Float,              32, 4, FormatKind.Float),
        //new(VertexFormat.RGB9E5Float,               32, 4, FormatKind.Float),
   ];

    public static ref readonly VertexAttributeFormatInfo GetFormatInfo(this VertexAttributeFormat format)
    {
        if (format >= VertexAttributeFormat.Count)
        {
            return ref s_vertexFormatInfos[0]; // Undefines
        }

        Guard.IsTrue(s_vertexFormatInfos[(uint)format].Format == format);
        return ref s_vertexFormatInfos[(uint)format];
    }

    public static int GetSizeInBytes(this VertexAttributeFormat format) => GetFormatInfo(format).ByteSize;
}
