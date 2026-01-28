// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _SHADER_DEFINITIONS_H_
#define _SHADER_DEFINITIONS_H_

#ifdef __cplusplus
#define ALIGNMENT alignas(16)
#else
#define ALIGNMENT
#endif

struct DispatchIndirectCommand
{
    uint x;
    uint y;
    uint z;
};

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};

struct DrawIndexedIndirectCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  baseVertex;
    uint firstInstance;
};

struct ALIGNMENT PerFrameData
{
    float elapsedTime;
    float totalTime;
};

struct ALIGNMENT PerViewData
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewMatrix;
    float4x4 inverseProjectionMatrix;
    float3 cameraPosition;
};

struct ALIGNMENT PBRMaterialData
{
    float4 baseColorFactor;
};

struct ALIGNMENT PerDrawData
{
    float4x4 worldMatrix;
    uint32_t materialIndex;
    //float4x4 inverseWorldMatrix;
};

#endif // _ALIMER_SHADER__
