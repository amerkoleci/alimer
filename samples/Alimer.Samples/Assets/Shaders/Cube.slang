#include "Alimer.hlsli"

struct VertexInput {
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
#ifdef VERTEX_COLOR
    float4 Color : COLOR;
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

#if defined(ALIMER_SPIRV)
sdasd
#elif defined(ALIMER_DXIL)
sdasd
#endif

//ALIMER_PUSH_CONSTANTS(DrawData, draw);
ConstantBuffer<DrawData> draw : register(b0, space0);
//ConstantBuffer<DrawData2> draw2 : register(b0, space1);
ConstantBuffer<DrawData2> draw2 : register(b0, space1);

[shader("vertex")]
VertexOutput vertexMain(in VertexInput input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    output.Position = mul(float4(input.Position, 1.0f), draw.worldMatrix);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
#ifdef VERTEX_COLOR
    output.Color = input.Color;
#else
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
    return output;
}

[shader("pixel")]
float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    return float4(input.Normal, 1.0f) * input.Color * draw2.color;
}
