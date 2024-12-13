#define CONCAT_X(a, b) a##b
#define CONCAT(a, b) CONCAT_X(a, b)

#if defined(VULKAN)
#   define VERTEX_ATTRIBUTE(type, name, loc) [[vk::location(loc)]] type name : CONCAT(ATTRIBUTE, loc)
#   define PUSH_CONSTANT(type, name, slot) [[vk::push_constant]] type name
#elif defined(HLSL5)
#   define PUSH_CONSTANT(type, name, slot) cbuffer name : register(CONCAT(b, slot)) { type name; }
#else
#   define VERTEX_ATTRIBUTE(type, name, loc) type name : CONCAT(ATTRIBUTE, loc)
#   define PUSH_CONSTANT(type, name, slot) ConstantBuffer<type> name : register(CONCAT(b, slot))
#endif

struct PushData {
    float4 color;
};

ConstantBuffer<PushData> data : register(b0, space0);
//PUSH_CONSTANT(PushData, data, 0);

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
    return input.Color * data.color;
}
