#include "Alimer.hlsli"
#include "VertexInputOutput.hlsli"
#include "ShaderTypes.h"

// Material (space 0)

// Draw - push constants
ALIMER_PUSH_CONSTANTS(PerDrawData, draw);

[shader("vertex")]
VertexOutput vertexMain(in VertexInput input)
{
    float4 worldPosition = mul(float4(input.Position, 1.0f), draw.worldMatrix);

    VertexOutput output;

    // Clip-space position
    output.Position = mul(worldPosition, view.viewProjectionMatrix);
    output.WorldPosition = worldPosition.xyz / worldPosition.w;
    // output.DepthVS = output.Position.w;

    // World-space normal
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), draw.worldMatrix)).xyz;

    // World-space tangent and bitangent
    //output.Tangent = normalize(mul(draw.worldMatrix, float4(input.Tangent.xyz, 0.0f))).xyz;
    //output.Bitangent = cross(output.Normal, output.Tangent) * input.Tangent.w;

    // Pass along the texture coordinates
    output.TexCoord0 = input.TexCoord0;
    output.TexCoord1 = float2(0.0f, 0.0f); // input.TexCoord1;

#if defined(VERTEX_COLOR)
    output.Color = input.Color;
#else
    output.Color = float3(1.0f);
#endif
    return output;
}
