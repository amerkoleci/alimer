// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics.VGPU;

internal readonly partial struct VGPUBool32 : IComparable, IComparable<VGPUBool32>, IEquatable<VGPUBool32>
{
    public readonly int Value;

    public VGPUBool32(int value)
    {
        Value = value;
    }

    public static VGPUBool32 True => new VGPUBool32(1);
    public static VGPUBool32 False => new VGPUBool32(0);

    public static bool operator ==(VGPUBool32 left, VGPUBool32 right) => left.Value == right.Value;

    public static bool operator !=(VGPUBool32 left, VGPUBool32 right) => left.Value != right.Value;

    public static bool operator <(VGPUBool32 left, VGPUBool32 right) => left.Value < right.Value;

    public static bool operator <=(VGPUBool32 left, VGPUBool32 right) => left.Value <= right.Value;

    public static bool operator >(VGPUBool32 left, VGPUBool32 right) => left.Value > right.Value;

    public static bool operator >=(VGPUBool32 left, VGPUBool32 right) => left.Value >= right.Value;

    public static implicit operator bool(VGPUBool32 value) => value.Value != 0;

    public static implicit operator VGPUBool32(bool value) => new VGPUBool32(value ? 1 : 0);

    public static bool operator false(VGPUBool32 value) => value.Value == 0;

    public static bool operator true(VGPUBool32 value) => value.Value != 0;

    public static implicit operator VGPUBool32(byte value) => new VGPUBool32(value);

    public static explicit operator byte(VGPUBool32 value) => (byte)(value.Value);

    public static implicit operator VGPUBool32(short value) => new VGPUBool32(value);

    public static explicit operator short(VGPUBool32 value) => (short)(value.Value);

    public static implicit operator VGPUBool32(int value) => new(value);

    public static implicit operator int(VGPUBool32 value) => value.Value;

    public static explicit operator VGPUBool32(long value) => new VGPUBool32(unchecked((int)(value)));

    public static implicit operator long(VGPUBool32 value) => value.Value;

    public static explicit operator VGPUBool32(nint value) => new VGPUBool32(unchecked((int)(value)));

    public static implicit operator nint(VGPUBool32 value) => value.Value;

    public static implicit operator VGPUBool32(sbyte value) => new VGPUBool32(value);

    public static explicit operator sbyte(VGPUBool32 value) => (sbyte)(value.Value);

    public static implicit operator VGPUBool32(ushort value) => new VGPUBool32(value);

    public static explicit operator ushort(VGPUBool32 value) => (ushort)(value.Value);

    public static explicit operator VGPUBool32(uint value) => new VGPUBool32(unchecked((int)(value)));

    public static explicit operator uint(VGPUBool32 value) => (uint)(value.Value);

    public static explicit operator VGPUBool32(ulong value) => new VGPUBool32(unchecked((int)(value)));

    public static explicit operator ulong(VGPUBool32 value) => (ulong)(value.Value);

    public static explicit operator VGPUBool32(nuint value) => new VGPUBool32(unchecked((int)(value)));

    public static explicit operator nuint(VGPUBool32 value) => (nuint)(value.Value);

    public int CompareTo(object? obj)
    {
        if (obj is VGPUBool32 other)
        {
            return CompareTo(other);
        }

        return (obj is null) ? 1 : throw new ArgumentException("obj is not an instance of Bool32.");
    }

    public int CompareTo(VGPUBool32 other) => Value.CompareTo(other.Value);

    public override bool Equals(object? obj) => (obj is VGPUBool32 other) && Equals(other);

    public bool Equals(VGPUBool32 other) => Value.Equals(other.Value);

    public override int GetHashCode() => Value.GetHashCode();

    public override string ToString() => Value != 0 ? "True" : "False";
}

