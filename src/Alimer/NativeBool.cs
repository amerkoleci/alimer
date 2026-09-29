// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;

namespace Alimer;

/// <summary>
/// One byte boolean value matching the C <c>bool</c> from <c>&lt;stdbool.h&gt;</c>.
/// </summary>
/// <param name="value"></param>
[StructLayout(LayoutKind.Sequential, Size = 1)]
public readonly partial struct NativeBool(byte value) : IComparable, IComparable<NativeBool>, IEquatable<NativeBool>
{
    public readonly byte Value = value;

    public static NativeBool True => new(1);
    public static NativeBool False => new(0);

    public static bool operator ==(NativeBool left, NativeBool right) => left.Value == right.Value;

    public static bool operator !=(NativeBool left, NativeBool right) => left.Value != right.Value;

    public static bool operator <(NativeBool left, NativeBool right) => left.Value < right.Value;

    public static bool operator <=(NativeBool left, NativeBool right) => left.Value <= right.Value;

    public static bool operator >(NativeBool left, NativeBool right) => left.Value > right.Value;

    public static bool operator >=(NativeBool left, NativeBool right) => left.Value >= right.Value;

    public static implicit operator bool(NativeBool value) => value.Value != 0;

    public static implicit operator NativeBool(bool value) => new(value ? (byte)1 : (byte)0);

    public static bool operator false(NativeBool value) => value.Value == 0;

    public static bool operator true(NativeBool value) => value.Value != 0;

    public static implicit operator NativeBool(byte value) => new(value);

    public static explicit operator byte(NativeBool value) => value.Value;

    public int CompareTo(object? obj)
    {
        if (obj is NativeBool other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException($"obj is not an instance of {nameof(NativeBool)}.");
    }

    public int CompareTo(NativeBool other) => Value.CompareTo(other.Value);

    public override bool Equals([NotNullWhen(true)] object? obj) => obj is NativeBool other && Equals(other);

    public bool Equals(NativeBool other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value != 0 ? "True" : "False";
}
