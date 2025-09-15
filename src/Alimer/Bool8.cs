// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

internal readonly partial struct Bool8(byte value) : IComparable, IComparable<Bool8>, IEquatable<Bool8>
{
    public readonly byte Value = value;

    public static Bool8 True => new(1);
    public static Bool8 False => new(0);

    public static bool operator ==(Bool8 left, Bool8 right) => left.Value == right.Value;

    public static bool operator !=(Bool8 left, Bool8 right) => left.Value != right.Value;

    public static bool operator <(Bool8 left, Bool8 right) => left.Value < right.Value;

    public static bool operator <=(Bool8 left, Bool8 right) => left.Value <= right.Value;

    public static bool operator >(Bool8 left, Bool8 right) => left.Value > right.Value;

    public static bool operator >=(Bool8 left, Bool8 right) => left.Value >= right.Value;

    public static implicit operator bool(Bool8 value) => value.Value != 0;

    public static implicit operator Bool8(bool value) => new(value ? (byte)1 : (byte)0);

    public static bool operator false(Bool8 value) => value.Value == 0;

    public static bool operator true(Bool8 value) => value.Value != 0;

    public static implicit operator Bool8(byte value) => new(value);

    public static explicit operator byte(Bool8 value) => value.Value;

    public int CompareTo(object? obj)
    {
        if (obj is Bool8 other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException($"obj is not an instance of {nameof(Bool8)}.");
    }

    public int CompareTo(Bool8 other) => Value.CompareTo(other.Value);

    public override bool Equals([NotNullWhen(true)] object? obj) => obj is Bool8 other && Equals(other);

    public bool Equals(Bool8 other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value != 0 ? "True" : "False";
}
