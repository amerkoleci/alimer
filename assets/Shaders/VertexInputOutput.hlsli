// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_VERTEX_INPUT_OUTPUT__
#define _ALIMER_VERTEX_INPUT_OUTPUT__

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif

struct VertexInput {
    float3 Position : ATTRIBUTE0;
    float3 Normal : ATTRIBUTE1;
    float2 TexCoord0 : ATTRIBUTE2;
    //float4 Tangent : ATTRIBUTE2;
    //float2 TexCoord0 : ATTRIBUTE3;

#if USE_VERTEX_COLOR
    float3 Color : ATTRIBUTE4;
#endif
};

struct VertexOutput {
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION; // world-space position
    //float DepthVS : DEPTHVS;
    float3 Normal : NORMAL;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
};

#endif // _ALIMER_VERTEX_INPUT_OUTPUT__
