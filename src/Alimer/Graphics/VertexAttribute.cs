// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public  record struct VertexAttribute
{
    public VertexFormat Format;
    public uint Offset;
    public uint ShaderLocation;

    public VertexAttribute(VertexFormat format, uint offset, uint shaderLocation)
    {
        Format = format;
        Offset = offset;
        ShaderLocation = shaderLocation;
    }
}
