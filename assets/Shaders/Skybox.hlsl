// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer.hlsli"
#include "ShaderTypes.h"
#include "VertexInputOutput.hlsli"
#include "AlimerBindless.hlsli"

[shader("vertex")]
VertexSkyboxOutput vertexMain(in VertexSkyboxInput input)
{
    VertexSkyboxOutput output;
    output.TexCoord = input.Position.xyz;

    // Rotate into view-space, centered on the camera
    float3 positionVS = mul(float4(input.Position, 0.0f), frame.viewMatrix).xyz;

    // Transform to clip-space
    output.Position = mul(float4(positionVS, 1.0f), frame.projectionMatrix);
    output.Position.z = output.Position.w;

    return output;
}

[shader("pixel")]
float4 fragmentMain(in VertexSkyboxOutput input) : SV_TARGET
{
    TextureCube<float4> environmentTexture = bindlessTextureCube[frame.EnvironmentTextureIndex];
    SamplerState environmentSampler = SamplerLinearWrap; // bindlessSamplers[frame.EnvironmentSamplerIndex];
    
    float3 color = environmentTexture.Sample(environmentSampler, normalize(input.TexCoord)).rgb;
    return float4(color, 1.0f);
}
