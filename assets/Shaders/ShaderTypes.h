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
    float2 _padding;
};

struct ALIGNMENT PerViewData
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewMatrix;
    float4x4 inverseProjectionMatrix;
    float3 cameraPosition;
    float _padding0;
    float3 ambientLight;
    uint activeLightCount;
};

enum class LightType
{
    Invalid,
    Directional,
    Point,
    Spot,
};

struct ALIGNMENT LightData
{
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
    float range;
    float innerConeCos;
    float outerConeCos;
    uint type;
};

/* TODO: pack and use half*/
struct ALIGNMENT PBRMaterialUniforms
{
    float4 baseColorFactor;
    float3 emissiveFactor;
    float _padding;
    float2 metallicRoughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
};

struct ALIGNMENT InstanceData
{
    float4x4 worldMatrix;
    //float4x4 inverseWorldMatrix;
    //float4 color;
    //uint32_t materialIndex;
};

#ifndef __cplusplus
// Frame (space 3)
ConstantBuffer<PerFrameData> frame : register(b0, space3);
TextureCube<float4> environmentTexture : register(t0, space3);
SamplerState environmentSampler : register(s0, space3);

// View (space 2)
ConstantBuffer<PerViewData> view : register(b0, space2);
StructuredBuffer<LightData> lights : register(t1, space2); // Until we fix D3D12 and Vulkan BindGroup (should be t0)

// Instance data + materials data (space 1)
StructuredBuffer<InstanceData> instanceDataBuffer : register(t0, space1);
#endif

#endif // _ALIMER_SHADER__
