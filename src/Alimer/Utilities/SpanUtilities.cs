// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Buffers.Binary;
using System.Runtime.CompilerServices;

namespace Alimer.Utilities;

/// <summary
/// >Provides an extensions methods for <see cref="Span{T}" /> and <see cref="ReadOnlySpan{T}" />.
/// </summary>
public static unsafe class SpanUtilities
{
    public static short ReadInt16(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadInt16LittleEndian(src);
    public static ushort ReadUInt16(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadUInt16LittleEndian(src);

    public static int ReadInt32(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadInt32LittleEndian(src);

    public static uint ReadUInt32(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadUInt32LittleEndian(src);

    public static long ReadInt64(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadInt64LittleEndian(src);

    public static ulong ReadUInt64(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadUInt64LittleEndian(src);

    public static Int128 ReadInt128(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadInt128LittleEndian(src);

    public static UInt128 ReadUInt128(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadUInt128LittleEndian(src);

    public static float ReadSingle(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadSingleLittleEndian(src);

    public static double ReadDouble(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadDoubleLittleEndian(src);

    public static Half ReadHalf(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadHalfLittleEndian(src);

    public static nint ReadIntPtr(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadIntPtrLittleEndian(src);
    public static nuint ReadUIntPtr(this ReadOnlySpan<byte> src) => BinaryPrimitives.ReadUIntPtrLittleEndian(src);

    public static Vector2 ReadVector2(this ReadOnlySpan<byte> src)
    {
        float x = BinaryPrimitives.ReadSingleLittleEndian(src);
        float y = BinaryPrimitives.ReadSingleLittleEndian(src[4..]);
        return new(x, y);
    }
}
