// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _SHADER_DEFINITIONS_H_
#define _SHADER_DEFINITIONS_H_

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

struct PerFrameData
{
    float elapsedTime;
    float totalTime;
    float2 _padding;
};

struct PerViewData
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

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

enum class LightType
{
    Directional,
    Point,
    Spot,
};

struct GPUInstance
{
    float4x4 WorldMatrix;
    int   MaterialIndex;                 // Index into material buffer
    int   VertexBufferIndex;             // Bindless handle to the vertex buffer
    int   BaseVertex;                    // Base vertex offset
    int   _pad;
};

struct GPULight
{
    float3 Position;    // World-space position (unused for directional)
    uint Type;          // 0=Directional, 1=Point, 2=Spot
    float3 Color;       // Linear RGB × intensity
    float Range;        // Effective falloff range
    float3 Direction;  // Normalized world-space direction (directional/spot)
    float _padding0;
    float InnerConeCos;
    float OuterConeCos;
    float2 _padding1;
};
//static_assert(sizeof(GPULight) == 64, "GPULight must be 64 bytes");

/* TODO: pack and use half*/
struct GPUMaterialPBR
{
    int baseIndex;
    int normalIndex;
    int metallicRoughnessIndex;
    int emissiveIndex;
    int occlusionIndex;
    int samplerIndex;
    float2 _padding0;
    float4 baseColorFactor;
    float3 emissiveFactor;
    float normalScale;
    float2 metallicRoughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
    int baseColorUVSet;
    int normalUVSet;
    int metallicRoughnessUVSet;
    int emissiveUVSet;
};

/* TODO: Handle in common and better way */
#if defined(ALIMER_SPIRV)
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<GPUInstance> bindlessGPUInstance[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<GPUMaterialPBR> bindlessGPUMaterial[];
#elif ALIMER_SHADER_MODEL >= 66
static const BindlessResource<StructuredBuffer<GPUInstance> > bindlessGPUInstance[];
static const BindlessResource<StructuredBuffer<GPUMaterialPBR> > bindlessGPUMaterial;
#else
#endif

#ifndef __cplusplus
// Frame (space 3)
ConstantBuffer<PerFrameData> frame : register(b0, space3);
TextureCube<float4> environmentTexture : register(t0, space3);
SamplerState environmentSampler : register(s0, space3);

// View (space 2)
ConstantBuffer<PerViewData> view : register(b0, space2);
StructuredBuffer<GPULight> lights : register(t1, space2); // Until we fix D3D12 and Vulkan BindGroup (should be t0)

// Instance data + materials data (space 1)
StructuredBuffer<GPUInstance> instanceDataBuffer : register(t0, space1);
#endif

#endif // _ALIMER_SHADER__
