// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// A 64-bit unsigned integer type appropriate for storing GPU addresses.
/// </summary>
public readonly partial struct GpuAddress(ulong value) : IComparable, IComparable<GpuAddress>, IEquatable<GpuAddress>, IFormattable
{
    public readonly ulong Value = value;

    public static bool operator ==(GpuAddress left, GpuAddress right) => left.Value == right.Value;

    public static bool operator !=(GpuAddress left, GpuAddress right) => left.Value != right.Value;

    public static bool operator <(GpuAddress left, GpuAddress right) => left.Value < right.Value;

    public static bool operator <=(GpuAddress left, GpuAddress right) => left.Value <= right.Value;

    public static bool operator >(GpuAddress left, GpuAddress right) => left.Value > right.Value;

    public static bool operator >=(GpuAddress left, GpuAddress right) => left.Value >= right.Value;

    public static implicit operator ulong(GpuAddress value) => value.Value;

    public static implicit operator GpuAddress(ulong value) => new(value);

    public int CompareTo(object? obj)
    {
        if (obj is GpuAddress other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException($"obj is not an instance of {nameof(GpuAddress)}.");
    }

    public int CompareTo(GpuAddress other) => Value.CompareTo(other.Value);

    public override bool Equals([NotNullWhen(true)] object? obj) => (obj is GpuAddress other) && Equals(other);

    public bool Equals(GpuAddress other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value.ToString();

    public string ToString(string? format, IFormatProvider? formatProvider) => Value.ToString(format, formatProvider);
}
