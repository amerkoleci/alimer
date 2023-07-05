struct VertexInput {
    float3 Position : ATTRIBUTE0;
    float4 Color    : ATTRIBUTE1;
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};

VertexOutput vertexMain(in VertexInput input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VertexOutput output;
    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    return output;
}

float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
#if VARIANT
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
#else
    return input.Color;
#endif
}
