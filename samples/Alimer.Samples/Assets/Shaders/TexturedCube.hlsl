#include "Alimer.hlsli"

struct VertexInput {
    float3 Position : ATTRIBUTE0;
    float3 Normal   : ATTRIBUTE1;
    float2 texcoord : ATTRIBUTE2;
#if defined(VERTEX_COLOR)
    float4 Color    : ATTRIBUTE3;
#endif
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 texcoord : TEXCOORD0;
    float4 Color : COLOR0;
};

struct DrawData
{
    float4x4 worldMatrix;
};

struct DrawData2
{
    float4 color;
};

//PUSH_CONSTANT(DrawData, draw, 0);
ConstantBuffer<DrawData> draw : register(b0, space0);

SamplerState pbrSampler : register(s0, space1);
Texture2D<float4> baseColorTexture : register(t0, space1);

VertexOutput vertexMain(in VertexInput input)
{
    VertexOutput output;
    output.Position = mul(float4(input.Position, 1.0f), draw.worldMatrix);
    output.Normal = input.Normal;
    output.texcoord = input.texcoord;
#if defined(VERTEX_COLOR)
    output.Color = input.Color;
#else
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    float4 baseColor = /*material.baseColorFactor **/ baseColorTexture.Sample(pbrSampler, input.texcoord);
    return baseColor;
}
