// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes vertex buffer layout <see cref="RenderPipelineDescriptor"/>.
/// </summary>
public record struct VertexBufferLayout
{
    public uint Stride;
    public VertexStepMode StepMode;
    public VertexAttribute[] Attributes;

    public VertexBufferLayout(params VertexAttribute[] attributes)
    {
        Attributes = attributes;

        uint computedStride = 0;
        for (int i = 0; i < attributes.Length; i++)
        {
            uint elementSize = (uint)attributes[i].Format.GetSizeInBytes();
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
}
