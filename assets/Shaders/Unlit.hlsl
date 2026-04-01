// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer.hlsli"
#include "BRDF.hlsli"
#include "ShaderTypes.h"
#include "VertexInputOutput.hlsli"
#include "AlimerBindless.hlsli"

// Material (space 0)
ConstantBuffer<PBRMaterialData> material : register(b0);
Texture2D<float4> baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

[shader("pixel")]
float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    return baseColor;
}
