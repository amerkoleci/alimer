// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;

namespace Alimer.Assets;

/// <summary>
/// Represents a unique identifier for an asset, encapsulating a globally unique identifier (GUID) value.
/// </summary>
public readonly struct AssetId : IComparable<AssetId>, IEquatable<AssetId>
{
    private readonly Guid _value;

    /// <summary>
    /// Gets a static instance of the AssetId that represents an empty identifier.
    /// </summary>
    public static AssetId Empty => new();

    /// <summary>
    /// Initializes a new instance of the AssetId class using the specified unique identifier.
    /// </summary>
    /// <param name="value">The GUID value that uniquely identifies the asset.</param>
    public AssetId(Guid value)
    {
        _value = value;
    }

    /// <summary>
    /// Initializes a new instance of the AssetId class using the specified unique identifier.
    /// </summary>
    /// <param name="value">The GUID value that uniquely identifies the asset.</param>
    public AssetId(string guid)
    {
        _value = new Guid(guid);
    }

    public static AssetId New() => new(Guid.NewGuid());

    /// <summary>
    /// Performs an explicit conversion from <see cref="Guid" /> to <see cref="AssetId"/>.
    /// </summary>
    /// <param name="value">The value to convert.</param>
    /// <returns>The result of the conversion.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static explicit operator AssetId(Guid guid) => new(guid);

    /// <summary>
    /// Performs an explicit conversion from <see cref="AssetId" /> to <see cref="Guid"/>.
    /// </summary>
    /// <param name="value">The value to convert.</param>
    /// <returns>The result of the conversion.</returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static explicit operator Guid(AssetId id) => id._value;

    /// <summary>
    /// Implements the ==.
    /// </summary>
    /// <param name="left">The left.</param>
    /// <param name="right">The right.</param>
    /// <returns>The result of the operator.</returns>
    public static bool operator ==(AssetId left, AssetId right) => left.Equals(right);

    /// <summary>
    /// Implements the !=.
    /// </summary>
    /// <param name="left">The left.</param>
    /// <param name="right">The right.</param>
    /// <returns>The result of the operator.</returns>
    public static bool operator !=(AssetId left, AssetId right) => !left.Equals(right);

    /// <inheritdoc/>
    public bool Equals(AssetId other) => _value == other._value;

    /// <inheritdoc/>
    public override bool Equals([NotNullWhen(true)] object? obj) => obj is AssetId value && Equals(value);

    /// <inheritdoc/>
    public override int GetHashCode() => _value.GetHashCode();

    /// <inheritdoc/>
    public int CompareTo(AssetId other) => _value.CompareTo(other._value);

    /// <inheritdoc/>
    public override string ToString() => _value.ToString();

    public static bool TryParse(string input, out AssetId result)
    {
        bool success = Guid.TryParse(input, out Guid guid);
        result = new AssetId(guid);
        return success;
    }

    public static AssetId Parse(string input) => new(Guid.Parse(input));
}
