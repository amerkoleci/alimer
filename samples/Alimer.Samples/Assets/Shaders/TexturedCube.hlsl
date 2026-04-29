#include "Alimer.hlsli"
//#include "AlimerBindless.hlsli"


#if defined(ALIMER_SPIRV)
/* TODO: Use VK_EXT_mutable_descriptor_type */
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 2;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 3;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER = 4;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2DArray bindlessTexture2DArray[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] TextureCube bindlessTextureCube[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTexture2D[];
#elif ALIMER_SHADER_MODEL >= 66
template<typename T>
struct BindlessResource
{
	T operator[](uint index) { return (T)ResourceDescriptorHeap[index]; }
};
template<>
struct BindlessResource<SamplerState>
{
	SamplerState operator[](uint index) { return (SamplerState)SamplerDescriptorHeap[index]; }
};
static const BindlessResource<SamplerState> bindlessSamplers;
static const BindlessResource<Texture2D> bindlessTexture2D;
static const BindlessResource<Texture2DArray> bindlessTexture2DArray;
static const BindlessResource<TextureCube> bindlessTextureCube;
static const BindlessResource<RWTexture2D<float4> > bindlessRWTexture2D;
#else
SamplerState bindlessSamplers[] : register(s0, space4);
Texture2D bindlessTexture2D[] : register(t0, space5);
#endif

struct VertexInput {
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
#if defined(VERTEX_COLOR)
    float4 Color : COLOR0;
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
    float4x4 worldViewProjectionMatrix;
};

ConstantBuffer<DrawData> draw : register(b0);

struct PushConstants
{
    int textureIndex;
};
ALIMER_PUSH_CONSTANTS(PushConstants);

[shader("vertex")]
VertexOutput vertexMain(in VertexInput input)
{
    VertexOutput output;
    output.Position = mul(float4(input.Position, 1.0f), draw.worldViewProjectionMatrix);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
#if defined(VERTEX_COLOR)
    output.Color = input.Color;
#else
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
    return output;
}

[shader("pixel")]
float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    Texture2D<float4> baseColorTexture = bindlessTexture2D[push.textureIndex];
    float4 baseColor = /*material.baseColorFactor **/baseColorTexture.Sample(SamplerPointClamp, input.TexCoord);
    return baseColor;
}
