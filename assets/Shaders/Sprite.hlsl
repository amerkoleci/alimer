#include "ShaderTypes.h"
#include "AlimerBindless.hlsli"

struct GPUSpriteDrawData
{
    float2 Position;
    float2 Scale;
    float2 SinCosRotation;
    float4 Color;
    float4 SourceRect;
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

ALIMER_PUSH_CONSTANTS(GPUSpriteBatchData);
ALIMER_STRUCTURED_BUFFER(GPUSpriteDrawData, bindlessGPUSpriteDrawData);

[shader("vertex")]
VertexOutput vertexMain(in uint vertexIndex : SV_VertexID, in uint instanceIndex : SV_InstanceID)
{
    float2 vtxPosition = 0.0f;
    if (vertexIndex == 1)
        vtxPosition = float2(1.0f, 0.0f);
    else if (vertexIndex == 2)
        vtxPosition = float2(1.0f, 1.0f);
    else if (vertexIndex == 3)
        vtxPosition = float2(0.0f, 1.0f);

    // Scale the quad so that it's texture-sized
    StructuredBuffer<GPUSpriteDrawData> spriteBuffer = bindlessGPUSpriteDrawData[push.SpriteBufferIndex];
    GPUSpriteDrawData instanceData = spriteBuffer[instanceIndex];
    float2 positionSS = vtxPosition * instanceData.SourceRect.zw;

    // Apply transforms in screen space
    float sinRotation = instanceData.SinCosRotation.x;
    float cosRotation = instanceData.SinCosRotation.y;
    positionSS *= instanceData.Scale;
    positionSS = mul(positionSS, float2x2(cosRotation, -sinRotation, sinRotation, cosRotation));
    positionSS += instanceData.Position;

    // Scale by the viewport size, flip Y, then rescale to device coordinates
    float2 positionDS = positionSS;
    positionDS /= push.ViewportSize;
    positionDS = positionDS * 2 - 1;
    positionDS.y *= -1;

    // Figure out the texture coordinates
    float2 outTexCoord = vtxPosition;
    outTexCoord.xy *= instanceData.SourceRect.zw / push.TextureSize;
    outTexCoord.xy += instanceData.SourceRect.xy / push.TextureSize;
    
    VertexOutput output;
    output.Position = float4(positionDS, 1.0f, 1.0f);
    output.TexCoord = outTexCoord;
    output.Color = instanceData.Color;
    return output;
}

[shader("pixel")]
float4 fragmentMain(in VertexOutput input) : SV_TARGET
{
    Texture2D<float4> spriteTexture = bindlessTexture2D[push.SpriteTextureIndex];
    SamplerState spriteSampler = SamplerLinearWrap; // bindlessSamplers[material.samplerIndex];
    return spriteTexture.Sample(spriteSampler, input.TexCoord) * input.Color;
}
