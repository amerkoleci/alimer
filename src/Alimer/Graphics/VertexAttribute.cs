// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public record struct VertexAttribute
{
    public VertexAttributeSemantic Semantic;
    public VertexFormat Format;
    public int Offset;
    public int SemanticIndex;

    public VertexAttribute(VertexAttributeSemantic semantic, VertexFormat format, int offset = 0, int semanticIndex = 0)
    {
        ArgumentOutOfRangeException.ThrowIfNegative(offset, nameof(offset));
        ArgumentOutOfRangeException.ThrowIfNegative(semanticIndex, nameof(semanticIndex));

        Semantic = semantic;
        Format = format;
        Offset = offset;
        SemanticIndex = semanticIndex;
    }
}
