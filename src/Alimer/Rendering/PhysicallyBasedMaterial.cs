// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// Standard PBR material.
/// </summary>
public sealed class PhysicallyBasedMaterial : UnlitMaterial
{
    public PhysicallyBasedMaterial()
        : base()
    {
    }

    public Texture? NormalTexture { get; set; }

    [DefaultValue(0.0f)]
    public float MetallicFactor { get; set; }

    [DefaultValue(1.0f)]
    public float RoughnessFactor { get; set; } = 1.0f;
    public Texture? MetallicRoughnessTexture { get; set; }
    public Vector3 EmissiveFactor { get; set; } = Vector3.Zero;
    public Texture? EmissiveTexture { get; set; }
    public Texture? OcclusionTexture { get; set; }

    [DefaultValue(1.0f)]
    public float OcclusionStrength { get; set; } = 1.0f;

    /// <inheritdoc/>
    protected override void Destroy()
    {
        NormalTexture?.Dispose();
        MetallicRoughnessTexture?.Dispose();
        EmissiveTexture?.Dispose();
        OcclusionTexture?.Dispose();
    }
}
