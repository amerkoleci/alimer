#include "Alimer.hlsli"
#include "ShaderTypes.h"

struct VertexInput {
    float3 Position : ATTRIBUTE0;
    float3 Normal : ATTRIBUTE1;
    float2 TexCoord0 : ATTRIBUTE2;
#if defined(VERTEX_COLOR)
    float3 Color    : ATTRIBUTE3;
#endif
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION; // world-space position
    float3 Normal : NORMAL;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
};

// Frame (space 2)
ConstantBuffer<PerFrameData> frame : register(b0, space2);

// View (space 1)
ConstantBuffer<PerViewData> view : register(b0, space1);

// Material (space 0)

// Draw - push constants
ALIMER_PUSH_CONSTANTS(PerDrawData, draw);
// ConstantBuffer<DrawData> draw : register(b0, space0);

ConstantBuffer<PBRMaterialData> material : register(b0);
Texture2D<float4> baseColorTexture : register(t0);

[shader("vertex")]
VertexOutput vertexMain(in VertexInput input)
{
    float4 worldPosition = mul(float4(input.Position, 1.0f), draw.worldMatrix);

    VertexOutput output;

    // Clip-space position
    output.Position = mul(worldPosition, view.viewProjectionMatrix);
    output.WorldPosition = worldPosition.xyz / worldPosition.w;
    // output.DepthVS = output.Position.w;

    // World-space normal
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), draw.worldMatrix)).xyz;

    // World-space tangent and bitangent
    //output.Tangent = normalize(mul(draw.worldMatrix, float4(input.Tangent.xyz, 0.0f))).xyz;
    //output.Bitangent = cross(output.Normal, output.Tangent) * input.Tangent.w;

    // Pass along the texture coordinates
    output.TexCoord0 = input.TexCoord0;
    output.TexCoord1 = float2(0.0f, 0.0f); // input.TexCoord1;

#if defined(VERTEX_COLOR)
    output.Color = input.Color;
#else
    output.Color = float3(1.0f);
#endif
    return output;
}

[shader("pixel")]
float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    float4 baseColor = material.baseColorFactor * baseColorTexture.Sample(SamplerLinearClamp, input.TexCoord0);
    return baseColor;
}
