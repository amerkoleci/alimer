#include "Alimer.hlsli"
#include "ShaderTypes.h"
#include "VertexInputOutput.hlsli"

enum class BRDF : uint {
    Lambert,
    TrowbridgeReitz,
};

static float3 CubeDirectionFromFaceAndUV(in int face, in float2 uv)
{
    float3 dir;
    switch (face)
    {
    case 0:
        dir = float3(1.0f, uv.y, -uv.x);
        break;
    case 1:
        dir = float3(-1.0f, uv.y, uv.x);
        break;
    case 2:
        dir = float3(uv.x, -1.0f, uv.y);
        break;
    case 3:
        dir = float3(uv.x, 1.0f, -uv.y);
        break;
    case 4:
        dir = float3(uv.x, uv.y, 1.0f);
        break;
    case 5:
        dir = float3(-uv.x, uv.y, -1.0f);
        break;
    }

    return normalize(dir);
}

static float2 EquirectUVFromCubeDirection(float3 v)
{
    const float2 scales = float2(0.1591549f, 0.3183099f);
    const float2 biases = float2(0.5f, 0.5f);
    // Assumes +Z is forward. For -X forward, use atan2(v.z, v.x) below instead.
    float2 uv = float2(atan2(-v.x, v.z), asin(v.y)) * scales + biases;
    return uv;
}

// Material (space 0)
Texture2D<float4> inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);
// SamplerState defaultSampler : register(s0);

// Utility kernel for converting from an equirectangular environment map (LDR or HDR) to a cube map
[shader("compute")]
[numthreads(32, 32, 1)]
void CubeFromEquirectangular(uint3 ThreadID: SV_DispatchThreadID)
{
    const SamplerState equirectSampler = SamplerLinearClamp;
    const uint2 coords = ThreadID.xy;
    const int face = ThreadID.z;

    float outputWidth, outputHeight, outputDepth;
    outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

    const float cubeSize = outputWidth;
    const float2 cubeUV = ((float2(ThreadID.xy) / (cubeSize - 1)) * 2 - 1);
    const float3 dir = CubeDirectionFromFaceAndUV(face, cubeUV);
    const float2 rectUV = EquirectUVFromCubeDirection(dir);
    float4 color = inputTexture.SampleLevel(equirectSampler, rectUV, 0);
    outputTexture[uint3(coords, face)] = color;
}
