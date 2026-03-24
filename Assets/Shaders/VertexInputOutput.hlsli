// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_VERTEX_INPUT_OUTPUT__
#define _ALIMER_VERTEX_INPUT_OUTPUT__

#ifndef USE_VERTEX_UV1
#define USE_VERTEX_UV1 0
#endif

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif

struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float2 TexCoord0 : TEXCOORD0;

#if USE_VERTEX_COLOR
    float2 TexCoord1 : TEXCOORD1;
#endif

#if USE_VERTEX_COLOR
    float3 Color : COLOR;
#endif
};

struct VertexOutput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : POSITION; // world-space position
    float3 Normal : NORMAL;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
};

struct VertexFullscreenQuadOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VertexSkyboxInput
{
    float3 Position : POSITION;
};

struct VertexSkyboxOutput
{
    float4 Position : SV_POSITION;
    float3 TexCoord : TEXCOORD0;
};

#endif // _ALIMER_VERTEX_INPUT_OUTPUT__
