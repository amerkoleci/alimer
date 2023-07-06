// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public readonly record struct VertexAttribute
{
    public VertexAttribute(VertexFormat format, uint offset, uint shaderLocation)
    {
        Format = format;
        Offset = offset;
        ShaderLocation = shaderLocation;
    }

    public VertexFormat Format { get; init; }
    public uint Offset { get; init; }
    public uint ShaderLocation { get; init; }
}
