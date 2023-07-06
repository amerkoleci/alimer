// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes vertex buffer layout <see cref="RenderPipelineDescription"/>.
/// </summary>
public readonly record struct VertexBufferLayout
{
    public VertexBufferLayout(params VertexAttribute[] attributes)
    {
        Attributes = attributes;

        uint computedStride = 0;
        for (int i = 0; i < attributes.Length; i++)
        {
            uint elementSize = attributes[i].Format.GetSizeInBytes();
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

    public VertexBufferLayout(uint stride, params VertexAttribute[] attributes)
    {
        Attributes = attributes;
        Stride = stride;
        StepMode = VertexStepMode.Vertex;
    }

    public VertexBufferLayout(uint stride, VertexStepMode stepMode, params VertexAttribute[] attributes)
    {
        Stride = stride;
        StepMode = stepMode;
        Attributes = attributes;
    }

    public uint Stride { get; init; }
    public VertexStepMode StepMode { get; init; }
    public VertexAttribute[] Attributes { get; init; }
}
