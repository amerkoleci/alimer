#include "Alimer.hlsli"
#include "VertexInputOutput.hlsli"
#include "ShaderTypes.h"

// Material (space 0)
ConstantBuffer<PBRMaterialData> material : register(b0);
Texture2D<float4> baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

[shader("pixel")]
float4 fragmentMain(in VertexOutput input, in bool isFrontFace: SV_IsFrontFace) : SV_TARGET
{
    // Vulkan has issues with static samplers
    // float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(baseColorSampler, input.TexCoord0);
    baseColor.rgb *= input.Color;

    return baseColor;
}
