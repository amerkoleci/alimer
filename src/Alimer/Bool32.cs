// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer;

internal readonly partial struct Bool32(uint value) : IComparable, IComparable<Bool32>, IEquatable<Bool32>
{
    public readonly uint Value = value;

    public static Bool32 True => new(1);
    public static Bool32 False => new(0);

    public static bool operator ==(Bool32 left, Bool32 right) => left.Value == right.Value;

    public static bool operator !=(Bool32 left, Bool32 right) => left.Value != right.Value;

    public static bool operator <(Bool32 left, Bool32 right) => left.Value < right.Value;

    public static bool operator <=(Bool32 left, Bool32 right) => left.Value <= right.Value;

    public static bool operator >(Bool32 left, Bool32 right) => left.Value > right.Value;

    public static bool operator >=(Bool32 left, Bool32 right) => left.Value >= right.Value;

    public static implicit operator bool(Bool32 value) => value.Value != 0;

    public static implicit operator Bool32(bool value) => new(value ? 1u : 0u);

    public static bool operator false(Bool32 value) => value.Value == 0;

    public static bool operator true(Bool32 value) => value.Value != 0;

    public static implicit operator Bool32(uint value) => new(value);

    public static explicit operator uint(Bool32 value) => value.Value;

    public int CompareTo(object? obj)
    {
        if (obj is Bool32 other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException($"obj is not an instance of {nameof(Bool32)}.");
    }

    public int CompareTo(Bool32 other) => Value.CompareTo(other.Value);

    public override bool Equals([NotNullWhen(true)] object? obj) => obj is Bool32 other && Equals(other);

    public bool Equals(Bool32 other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value != 0 ? "True" : "False";
}
