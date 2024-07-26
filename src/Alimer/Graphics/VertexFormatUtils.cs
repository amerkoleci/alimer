// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public readonly record struct VertexFormatInfo(
    VertexFormat Format,
    uint ByteSize,
    uint ComponentCount,
    uint ComponentByteSize,
    FormatKind Kind
);

public static class VertexFormatUtils
{
    private static readonly VertexFormatInfo[] s_vertexFormatInfos =
   [
        new(VertexFormat.Undefined,                 0, 0, 0,  FormatKind.Float),
        new(VertexFormat.UByte2,                    2, 2, 1,  FormatKind.Uint),
        new(VertexFormat.UByte4,                    4, 4, 1,  FormatKind.Uint),
        new(VertexFormat.Byte2,                     2, 2, 1,  FormatKind.Sint),
        new(VertexFormat.Byte4,                     4, 4, 1,  FormatKind.Sint),
        new(VertexFormat.UByte2Normalized,          2, 2, 1,  FormatKind.Unorm),
        new(VertexFormat.UByte4Normalized,          4, 4, 1,  FormatKind.Unorm),
        new(VertexFormat.Byte2Normalized,           2, 2, 1,  FormatKind.Snorm),
        new(VertexFormat.Byte4Normalized,           4, 4, 1,  FormatKind.Snorm),
        new(VertexFormat.UShort2,                   4, 2, 2,  FormatKind.Uint),
        new(VertexFormat.UShort4,                   8, 4, 2,  FormatKind.Uint),
        new(VertexFormat.Short2,                    4, 2, 2,  FormatKind.Sint),
        new(VertexFormat.Short4,                    8, 4, 2,  FormatKind.Sint),
        new(VertexFormat.UShort2Normalized,         4, 2, 2,  FormatKind.Unorm),
        new(VertexFormat.UShort4Normalized,         8, 4, 2,  FormatKind.Unorm),
        new(VertexFormat.Short2Normalized,          4, 2, 2,  FormatKind.Snorm),
        new(VertexFormat.Short4Normalized,          8, 4, 2,  FormatKind.Snorm),
        new(VertexFormat.Half2,                     4, 2, 2,  FormatKind.Float),
        new(VertexFormat.Half4,                     8, 4, 2,  FormatKind.Float),
        new(VertexFormat.Float,                     4, 1, 4,  FormatKind.Float),
        new(VertexFormat.Float2,                    8, 2, 4,  FormatKind.Float),
        new(VertexFormat.Float3,                    12, 3, 4, FormatKind.Float),
        new(VertexFormat.Float4,                    16, 4, 4, FormatKind.Float),
        new(VertexFormat.UInt,                      4, 1, 4,  FormatKind.Uint),
        new(VertexFormat.UInt2,                     8, 2, 4,  FormatKind.Uint),
        new(VertexFormat.UInt3,                     12, 3, 4, FormatKind.Uint),
        new(VertexFormat.UInt4,                     16, 4, 4, FormatKind.Uint),
        new(VertexFormat.Int,                       4, 1, 4,  FormatKind.Sint),
        new(VertexFormat.Int2,                      8, 2, 4,  FormatKind.Sint),
        new(VertexFormat.Int3,                      12, 3, 4, FormatKind.Sint),
        new(VertexFormat.Int4,                      16, 4, 4, FormatKind.Sint),

        //new(VertexFormat.Int1010102Normalized,    32, 4, 4, FormatKind.Unorm),
        new(VertexFormat.UInt1010102Normalized,     32, 4, 4, FormatKind.Uint),
        new(VertexFormat.RG11B10Float,              32, 4, 4, FormatKind.Float),
        new(VertexFormat.RGB9E5Float,               32, 4, 4, FormatKind.Float),
   ];

    public static ref readonly VertexFormatInfo GetFormatInfo(this VertexFormat format)
    {
        if (format >= VertexFormat.Count)
        {
            return ref s_vertexFormatInfos[0]; // Undefines
        }

        Guard.IsTrue(s_vertexFormatInfos[(uint)format].Format == format);
        return ref s_vertexFormatInfos[(uint)format];
    }

    public static uint GetSizeInBytes(this VertexFormat format) => GetFormatInfo(format).ByteSize;
}
