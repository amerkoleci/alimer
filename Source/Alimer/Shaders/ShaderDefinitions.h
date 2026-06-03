// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _SHADER_DEFINITIONS_H_
#define _SHADER_DEFINITIONS_H_

#ifdef __cplusplus
using bool32 = uint32_t;
using float2 = Alimer::Vector2;
using float3 = Alimer::Vector3;
using float4 = Alimer::Vector4;
using color4 = Alimer::Color;
using int4 = Alimer::Int4;
using uint2 = Alimer::UInt2;
using uint4 = Alimer::UInt4;
//using float3x3 = Alimer::Matrix3x3;
using float4x4 = Alimer::Matrix4x4;
#else
#define constexpr
#define bool32 bool
#define color4 float4
#define alignas(x)
#endif

struct DispatchIndirectCommand
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct DrawIndirectCommand
{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedIndirectCommand
{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  baseVertex;
    uint32_t firstInstance;
};

#endif
