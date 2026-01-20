// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// A 64-bit unsigned integer type appropriate for storing GPU addresses.
/// </summary>
public readonly partial struct GPUAddress(ulong value) : IComparable, IComparable<GPUAddress>, IEquatable<GPUAddress>, IFormattable
{
    public readonly ulong Value = value;

    public static bool operator ==(GPUAddress left, GPUAddress right) => left.Value == right.Value;

    public static bool operator !=(GPUAddress left, GPUAddress right) => left.Value != right.Value;

    public static bool operator <(GPUAddress left, GPUAddress right) => left.Value < right.Value;

    public static bool operator <=(GPUAddress left, GPUAddress right) => left.Value <= right.Value;

    public static bool operator >(GPUAddress left, GPUAddress right) => left.Value > right.Value;

    public static bool operator >=(GPUAddress left, GPUAddress right) => left.Value >= right.Value;

    public static implicit operator ulong(GPUAddress value) => value.Value;

    public static implicit operator GPUAddress(ulong value) => new(value);

    public int CompareTo(object? obj)
    {
        if (obj is GPUAddress other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException($"obj is not an instance of {nameof(GPUAddress)}.");
    }

    public int CompareTo(GPUAddress other) => Value.CompareTo(other.Value);

    public override bool Equals([NotNullWhen(true)] object? obj) => (obj is GPUAddress other) && Equals(other);

    public bool Equals(GPUAddress other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value.ToString();

    public string ToString(string? format, IFormatProvider? formatProvider) => Value.ToString(format, formatProvider);
}
