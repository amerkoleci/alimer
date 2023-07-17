#include "Alimer.hlsli"

struct VertexInput {
    float3 Position : ATTRIBUTE0;
    float3 Normal   : ATTRIBUTE1;
    float2 TexCoord : ATTRIBUTE2;
#if defined(VERTEX_COLOR)
    float4 Color    : ATTRIBUTE3;
#endif
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
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
Texture2D<float4> Texture : register(t0, space0);

VertexOutput vertexMain(in VertexInput input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    output.Position = mul(draw.worldMatrix, float4(input.Position, 1.0f));
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
#if defined(VERTEX_COLOR)
    output.Color = input.Color;
#else
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    return Texture.Sample(SamplerPointWrap, input.TexCoord);
}
