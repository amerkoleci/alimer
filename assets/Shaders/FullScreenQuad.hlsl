// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/

#include "VertexInputOutput.hlsli"

[shader("vertex")]
VertexFullscreenQuadOutput vertexMain(uint vertexIndex : SV_VertexID)
{
    VertexFullscreenQuadOutput output;
    output.TexCoord = float2((vertexIndex << 1) & 2, vertexIndex & 2);
    output.Position = float4(output.TexCoord * 2.0f + -1.0f, 0.0f, 1.0f);
    return output;
}
