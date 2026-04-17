#include "Alimer.hlsli"
#include <Alimer/Shaders/ShaderDefinitions.h>

struct VertexInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct PushData
{
    float4x4 worldViewProjectionMatrix;
};
//ALIMER_PUSH_CONSTANTS(PushData);
//ConstantBuffer<PushData> push : register(b0);
ALIMER_PUSH_CONSTANTS(PushData, push);

VertexOutput vertexMain(in VertexInput input)
{
    VertexOutput output;
    output.Position = mul(float4(input.Position, 1.0f), push.worldViewProjectionMatrix);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    return float4(input.Normal, 1.0f);
}
