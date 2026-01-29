// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public struct MeshVertexBufferLayout
{
    public int Stride;
    public VertexStepMode StepMode;
    public MeshVertexAttribute[] Attributes;                    

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

    public readonly bool HasAttribute(MeshVertexAttributeSemantic semantic)
    {
        for (int i = 0; i < Attributes.Length; i++)
        {
            if (Attributes[i].Semantic == semantic)
                return true;
        }

        return false;
    }

    public readonly MeshVertexAttribute? GetAttribute(MeshVertexAttributeSemantic semantic)
    {
        for (int i = 0; i < Attributes.Length; i++)
        {
            if (Attributes[i].Semantic == semantic)
                return Attributes[i];
        }

        return default;
    }

    public readonly int GetAttributeOffset(MeshVertexAttributeSemantic semantic)
    {
        for (int i = 0; i < Attributes.Length; i++)
        {
            if (Attributes[i].Semantic == semantic)
                return Attributes[i].Offset;
        }

        return -1;
    }

    public readonly void GetAttributes(Span<MeshVertexAttribute> attributes)
    {
        if (attributes.Length < Attributes.Length)
        {
            throw new ArgumentException("The provided span is too small to hold all attributes.", nameof(attributes));
        }

        for (int i = 0; i < Attributes.Length; i++)
        {
            attributes[i] = Attributes[i];
        }
    }
}
