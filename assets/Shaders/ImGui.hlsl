// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

// Input from vertex buffer
struct VSInput {
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

// Output from vertex shader / Input to fragment shader
struct VSOutput {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

// Push constants for transformation
struct PushConstants {
    float2 Scale;
    float2 Translate;
};

ALIMER_PUSH_CONSTANTS(PushConstants);

SamplerState imguiSampler : register(s0);
Texture2D<float4> imguiTexture : register(t0);

[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    VSOutput output;

    // Transform position
    output.Position = float4(input.Position * push.Scale + push.Translate, 0.0, 1.0);

    // Pass UV and color to fragment shader
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;

    return output;
}

[shader("pixel")]
float4 fragmentMain(VSOutput input) : SV_TARGET
{
    // Sample font texture and multiply by color
    float4 color = input.Color * imguiTexture.Sample(imguiSampler, input.TexCoord);
    return color;
}
