// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

/// <summary>
/// Unlit material.
/// </summary>
public class UnlitMaterial : Material
{
    public UnlitMaterial()
        : base()
    {
    }

    public Color BaseColorFactor { get; set; } = new(1.0f, 1.0f, 1.0f, 1.0f);
    public Texture? BaseColorTexture { get; set; }

    public bool DoubleSided { get; set; }

    public float AlphaCutoff { get; set; }

    public bool DepthWrite { get; set; } = true;

    public CompareFunction DepthCompare { get; set; } = CompareFunction.Less;

    public bool AdditiveBlend { get; set; }

    //public bool CastsShadow { get; set; } = true;

    /// <inheritdoc cref="Dispose()" />
    /// <param name="disposing"><c>true</c> if the method was called from <see cref="Dispose()" />; otherwise, <c>false</c>.</param>
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            BaseColorTexture?.Dispose();
        }
    }
}
