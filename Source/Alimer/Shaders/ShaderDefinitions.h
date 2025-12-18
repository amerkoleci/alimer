// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _SHADER_DEFINITIONS_H_
#define _SHADER_DEFINITIONS_H_

#define PER_FRAME_BIND_GROUP 0
#define PER_VIEW_BIND_GROUP 1
#define PER_DRAW_BIND_GROUP 2
#define PER_MATERIAL_BIND_GROUP 3

#define MAX_BIND_GROUPS 4
#define MAX_PUSH_CONSTANTS_SIZE 128 
#define START_BINDLESS_SPACE 1000

#ifdef __cplusplus
using bool32 = uint32_t;
using uint = uint32_t;
//using float2 = Alimer::Vector2;
//using float3 = Alimer::Vector3;
//using float4 = Alimer::Vector4;
//using color4 = Alimer::Color;
//using int4 = Alimer::Int4;
//using uint2 = Alimer::UInt2;
//using uint4 = Alimer::UInt4;
//using float3x3 = Alimer::Matrix3x3;
//using float4x4 = Alimer::Matrix4x4;
#else
#define constexpr
#define bool32 bool
#define color4 float4
#define alignas(x)
#endif

// Static samplers
static constexpr uint32_t kImmutableSamplerSlotBegin = 100;

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

#endif
