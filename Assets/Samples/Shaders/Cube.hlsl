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
    float4x4 worldMatrix;
    float4x4 viewProjectionMatrix;
};
ALIMER_PUSH_CONSTANTS(PushData);

VertexOutput vertexMain(in VertexInput input)
{
    float4 worldPosition = mul(float4(input.Position, 1.0f), push.worldMatrix);
    
    VertexOutput output;
    output.Position = mul(worldPosition, push.viewProjectionMatrix);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    return float4(input.Normal, 1.0f);
}
