#include "Alimer.hlsli"
#include "VertexInputOutput.hlsli"
#include "ShaderTypes.h"

struct SurfaceInfo
{
    // Fill these yourself:
    float3 worldPos; // world space position
    float3 V;        // normalized vector from the shading location to the eye
    float3 N;        // surface normal in the world space

    float roughness;
    float metalness;
    float3 specularColor;
    float3 diffuseColor;

    float ao;
    float alpha;
    float3 emissive;
    float4 transmission;

    // Calculated properties
    float NdotV;
    float3 R;
    float3 F0;
};

// Material (space 0)
ConstantBuffer<PBRMaterialData> material : register(b0);
Texture2D<float4> baseColorTexture : register(t0);
SamplerState baseColorSampler : register(s0);

[shader("pixel")]
float4 fragmentMain(in VertexOutput input, in bool isFrontFace: SV_IsFrontFace) : SV_TARGET
{
    const float4 baseColorMap = baseColorTexture.Sample(baseColorSampler, input.TexCoord0);

    // float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    float4 baseColor = material.baseColorFactor * baseColorMap;
    baseColor.rgb *= input.Color;
    // clip(baseColor.a - material.GetAlphaCutoff());

    SurfaceInfo surface;
    surface.worldPos = input.WorldPosition;
    surface.V = normalize(view.cameraPosition.xyz - surface.worldPos);

    // Should we normalize here?
    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);

    return baseColor;
}
