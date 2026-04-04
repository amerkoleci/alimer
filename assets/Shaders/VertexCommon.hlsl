#include "Alimer.hlsli"
#include "VertexInputOutput.hlsli"
#include "ShaderTypes.h"
#include "AlimerBindless.hlsli"

[shader("vertex")]
VertexOutput vertexMain(in VertexInput input, uint instanceId: SV_InstanceID)
{
    StructuredBuffer<GPUInstance> instances = bindlessGPUInstance[push.InstanceBufferIndex];
    GPUInstance instance = instances[instanceId];

    float4x4 worldMatrix = instance.WorldMatrix;
    //const float4 instanceColor = instance.color;
    float4 worldPosition = mul(float4(input.Position, 1.0f), worldMatrix);

    VertexOutput output;

    // Clip-space position
    output.Position = mul(worldPosition, frame.viewProjectionMatrix);
    output.WorldPosition = worldPosition.xyz / worldPosition.w;

    // World-space normal
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), worldMatrix)).xyz;

    // World-space tangent and bitangent
    output.Tangent = normalize(mul(float4(input.Tangent.xyz, 0.0f), worldMatrix)).xyz;
    output.Bitangent = cross(output.Normal, output.Tangent) * input.Tangent.w;

    // Pass along the texture coordinates
    output.TexCoord0 = input.TexCoord0;

#if USE_VERTEX_COLOR
    output.TexCoord1 = input.TexCoord1;
#else
    output.TexCoord1 = float2(0.0f, 0.0f);
#endif

#if USE_VERTEX_COLOR
    output.Color = input.Color;
#else
    // output.Color = instanceColor.xyz;
    output.Color = float3(1.0f, 1.0f, 1.0f);
#endif

    output.MaterialIndex = instance.MaterialIndex;

    return output;
}
