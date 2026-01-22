// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public readonly struct MeshVertexBufferLayout
{
    public int Stride { get; init; }
    public VertexStepMode StepMode { get; init; }
    public MeshVertexAttribute[] Attributes { get; init; }

    public MeshVertexBufferLayout(params Span<MeshVertexAttribute> attributes)
    {
        Attributes = attributes.ToArray();

        int computedStride = 0;
        for (int i = 0; i < attributes.Length; i++)
        {
            int elementSize = (int)attributes[i].Format.GetSizeInBytes();
            if (attributes[i].Offset != 0)
            {
                computedStride = attributes[i].Offset + elementSize;
            }
            else
            {
                computedStride += elementSize;
            }
        }

        Stride = computedStride;
        StepMode = VertexStepMode.Vertex;
    }
}
