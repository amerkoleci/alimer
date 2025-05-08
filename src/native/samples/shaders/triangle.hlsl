#include "Alimer.hlsli"

struct PushData {
    float4 color;
    uint textureIndex;
};

//ConstantBuffer<PushData> data : register(b0, space0);
ALIMER_PUSH_CONSTANTS(PushData, data, 0);

struct VertexInput {
    float3 Position     : ATTRIBUTE0;
    float4 Color        : ATTRIBUTE1;
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VertexOutput vertexMain(in VertexInput input)
{
    VertexOutput output;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
#if TEST_BINDLESS
    Texture2D baseColorTexture = bindlessTexture2D[data.textureIndex];
    SamplerState baseColorSampler = SamplerPointClamp; // bindlessSamplers[data.textureIndex];
    float4 baseColor = baseColorTexture.Sample(baseColorSampler, float2(0.0, 0.0f));
    return input.Color * data.color * baseColor;
#else
    return input.Color * data.color;
#endif
}
