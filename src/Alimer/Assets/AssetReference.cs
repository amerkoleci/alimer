// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets;

public sealed class AssetReference : IEquatable<AssetReference>
{
    /// <summary>
    /// Initializes a new instance of the <see cref="AssetReference"/> class.
    /// </summary>
    /// <param name="id">The unique identifier of the asset.</param>
    /// <param name="location">The location.</param>
    public AssetReference(AssetId id, string location)
    {
        Id = id;
        Location = location;
    }

    /// <summary>
    /// Gets or sets the unique identifier of the reference asset.
    /// </summary>
    /// <value>The unique identifier of the reference asset..</value>
    public AssetId Id { get; init; }

    /// <summary>
    /// Gets or sets the location of the asset.
    /// </summary>
    /// <value>The location.</value>
    public string Location { get; init; }

    public bool Equals(AssetReference? other)
    {
        if (other is null) return false;
        if (ReferenceEquals(this, other)) return true;
        return Equals(Location, other.Location) && Id.Equals(other.Id);
    }
}
