struct VertexInput {
    float3 Position : POSITION;
    float4 Color    : COLOR;
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
#ifdef VARIANT
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
#else
    return input.Color;
#endif
}
