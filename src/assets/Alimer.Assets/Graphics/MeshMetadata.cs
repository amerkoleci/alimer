// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Assets.Graphics;

public sealed class MeshMetadata : AssetMetadata
{
    /// <summary>
    /// Gets or sets a value indicating whether tangents are generated during mesh processing.
    /// </summary>
    /// <remarks>When set to <see langword="true"/>, tangents will be generated, which may affect the output and performance of mesh processing.
    /// The default value is <see langword="true"/>.</remarks>
    public bool GenerateTangets { get; set; } = true;
}
