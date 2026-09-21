// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Define a <see cref="VertexAttribute"/> format.
/// </summary>
public enum VertexAttributeFormat
{
    /// <summary>
    /// An 8-bit, unsigned integer value.
    /// </summary>
    UByte,
    /// <summary>
    /// A two-component vector with 8-bit, unsigned integer values.
    /// </summary>
    UByte2,
    /// <summary>
    /// A four-component vector with 8-bit, unsigned integer values.
    /// </summary>
    UByte4,
    /// <summary>
    /// An 8-bit, signed integer value.
    /// </summary>
    Byte,
    /// <summary>
    /// A two-component vector with 8-bit, signed integer values.
    /// </summary>
    Byte2,
    /// <summary>
    /// A four-component vector with 8-bit, signed integer values.
    /// </summary>
    Byte4,
    /// <summary>
    /// An 8-bit, normalized, unsigned integer value.
    /// </summary>
    UByteNormalized,
    /// <summary>
    /// A two-component vector with 8-bit, normalized, unsigned integer values.
    /// </summary>
    UByte2Normalized,
    /// <summary>
    /// A four-component vector with 8-bit, normalized, unsigned integer values.
    /// </summary>
    UByte4Normalized,
    /// <summary>
    /// An 8-bit, normalized, signed integer value.
    /// </summary>
    ByteNormalized,
    /// <summary>
    /// A two-component vector with 8-bit, normalized, signed integer values.
    /// </summary>
    Byte2Normalized,
    /// <summary>
    /// A four-component vector with 8-bit, normalized, signed integer values.
    /// </summary>
    Byte4Normalized,
    /// <summary>
    /// A 16-bit, unsigned integer value.
    /// </summary>
    UShort,
    /// <summary>
    /// A two-component vector with 16-bit, unsigned integer values.
    /// </summary>
    UShort2,
    /// <summary>
    /// A four-component vector with 16-bit, unsigned integer values.
    /// </summary>
    UShort4,
    /// <summary>
    /// A 16-bit, signed integer value.
    /// </summary>
    Short,
    /// <summary>
    /// A two-component vector with 16-bit, signed integer values.
    /// </summary>
    Short2,
    /// <summary>
    /// A four-component vector with 16-bit, signed integer values.
    /// </summary>
    Short4,
    /// <summary>
    /// A 16-bit, normalized, unsigned integer value.
    /// </summary>
    UShortNormalized,
    /// <summary>
    /// A two-component vector with 16-bit, normalized, unsigned integer values.
    /// </summary>
    UShort2Normalized,
    /// <summary>
    /// A four-component vector with 16-bit, normalized, unsigned integer values.
    /// </summary>
    UShort4Normalized,
    /// <summary>
    /// A 16-bit, normalized, signed integer value.
    /// </summary>
    ShortNormalized,
    /// <summary>
    /// A two-component vector with 16-bit, normalized, signed integer values.
    /// </summary>
    Short2Normalized,
    /// <summary>
    /// A four-component vector with 16-bit, normalized, signed integer values.
    /// </summary>
    Short4Normalized,
    /// <summary>
    /// A 16-bit floating-point value.
    /// </summary>
    Half,
    /// <summary>
    /// A two-component vector with 16-bit floating-point values.
    /// </summary>
    Half2,
    /// <summary>
    /// A four-component vector with 16-bit floating-point values.
    /// </summary>
    Half4,
    /// <summary>
    /// A 32-bit floating-point value.
    /// </summary>
    Float,
    /// <summary>
    /// A two-component vector with 32-bit floating-point values.
    /// </summary>
    Float2,
    /// <summary>
    /// A three-component vector with 32-bit floating-point values.
    /// </summary>
    Float3,
    /// <summary>
    /// A four-component vector with 32-bit floating-point values.
    /// </summary>
    Float4,
    /// <summary>
    /// A 32-bit, unsigned integer value.
    /// </summary>
    UInt,
    /// <summary>
    /// A two-component vector with 32-bit, unsigned integer values.
    /// </summary>
    UInt2,
    /// <summary>
    /// A three-component vector with 32-bit, unsigned integer values.
    /// </summary>
    UInt3,
    /// <summary>
    /// A four-component vector with 32-bit, unsigned integer values.
    /// </summary>
    UInt4,
    /// <summary>
    /// A 32-bit unsigned integer value.
    /// </summary>
    Int,
    /// <summary>
    /// A two-component vector with 32-bit, signed integer values.
    /// </summary>
    Int2,
    /// <summary>
    /// A three-component vector with 32-bit, signed integer values.
    /// </summary>
    Int3,
    /// <summary>
    /// A four-component vector with 32-bit, signed integer values.
    /// </summary>
    Int4,

    //Int1010102Normalized,
    Unorm10_10_10_2,
    //Snorm10_10_10_2,
    Unorm8x4BGRA,
    //RG11B10Float,
    //RGB9E5Float,


    Count
}
