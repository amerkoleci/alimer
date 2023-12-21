// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using Vortice.Mathematics;

namespace Alimer.Rendering;

/// <summary>
/// Unlit material.
/// </summary>
public class UnlitMaterial : Material
{
    public UnlitMaterial(GraphicsDevice device)
        : base(device)
    {
    }
    /// <summary>
    /// Finalizes an instance of the <see cref="UnlitMaterial" /> class.
    /// </summary>
    ~UnlitMaterial() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }
    }

    public Color4 BaseColorFactor { get; set; } = new (1.0f, 1.0f, 1.0f, 1.0f);
    public Texture? BaseColorTexture { get; set; }

    public bool Unlit { get; set; }

    public bool Transparent { get; set; }
    public bool DoubleSided { get; set; }

    public float AlphaCutoff { get; set; }

    public bool DepthWrite { get; set; } = true;

    public CompareFunction DepthCompare { get; set; } = CompareFunction.Less;

    public bool AdditiveBlend { get; set; } 
    //public bool CastsShadow { get; set; } = true;
}
