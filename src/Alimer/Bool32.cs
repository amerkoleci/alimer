// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public readonly partial struct Bool32(int value) : IComparable, IComparable<Bool32>, IEquatable<Bool32>
{
    public readonly int Value = value;

    public static Bool32 True => new(1);
    public static Bool32 False => new(0);

    public static bool operator ==(Bool32 left, Bool32 right) => left.Value == right.Value;

    public static bool operator !=(Bool32 left, Bool32 right) => left.Value != right.Value;

    public static bool operator <(Bool32 left, Bool32 right) => left.Value < right.Value;

    public static bool operator <=(Bool32 left, Bool32 right) => left.Value <= right.Value;

    public static bool operator >(Bool32 left, Bool32 right) => left.Value > right.Value;

    public static bool operator >=(Bool32 left, Bool32 right) => left.Value >= right.Value;

    public static implicit operator bool(Bool32 value) => value.Value != 0;

    public static implicit operator Bool32(bool value) => new(value ? 1 : 0);

    public static bool operator false(Bool32 value) => value.Value == 0;

    public static bool operator true(Bool32 value) => value.Value != 0;

    public static implicit operator Bool32(byte value) => new(value);

    public static explicit operator byte(Bool32 value) => (byte)(value.Value);

    public static implicit operator Bool32(short value) => new(value);

    public static explicit operator short(Bool32 value) => (short)(value.Value);

    public static implicit operator Bool32(int value) => new(value);

    public static implicit operator int(Bool32 value) => value.Value;

    public static explicit operator Bool32(long value) => new(unchecked((int)(value)));

    public static implicit operator long(Bool32 value) => value.Value;

    public static explicit operator Bool32(nint value) => new(unchecked((int)(value)));

    public static implicit operator nint(Bool32 value) => value.Value;

    public static implicit operator Bool32(sbyte value) => new(value);

    public static explicit operator sbyte(Bool32 value) => (sbyte)(value.Value);

    public static implicit operator Bool32(ushort value) => new(value);

    public static explicit operator ushort(Bool32 value) => (ushort)(value.Value);

    public static explicit operator Bool32(uint value) => new(unchecked((int)(value)));

    public static explicit operator uint(Bool32 value) => (uint)(value.Value);

    public static explicit operator Bool32(ulong value) => new(unchecked((int)(value)));

    public static explicit operator ulong(Bool32 value) => (ulong)(value.Value);

    public static explicit operator Bool32(nuint value) => new(unchecked((int)(value)));

    public static explicit operator nuint(Bool32 value) => (nuint)(value.Value);

    public int CompareTo(object? obj)
    {
        if (obj is Bool32 other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException("obj is not an instance of Bool32.");
    }

    public int CompareTo(Bool32 other) => Value.CompareTo(other.Value);

    public override bool Equals(object? obj) => (obj is Bool32 other) && Equals(other);

    public bool Equals(Bool32 other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value != 0 ? "True" : "False";
}
