// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using System.Numerics;
using Alimer.Graphics;

namespace Alimer.Rendering;

public enum MeshVertexAttributeSemantic
{
    Position,
    Normal,
    Tangent,
    Color,
    BlendIndices,
    BlendWeights,
    TexCoord0,
    TexCoord1,
    TexCoord2,
    TexCoord3,
    TexCoord4,
    TexCoord5,
    TexCoord6,
    TexCoord7,

    Custom
}

public struct MeshVertexAttribute(MeshVertexAttributeSemantic semantic, VertexFormat format, int offset)
{
    public MeshVertexAttributeSemantic Semantic = semantic;
    public VertexFormat Format = format;
    public int Offset = offset;
}
