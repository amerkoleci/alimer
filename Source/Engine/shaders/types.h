// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _SHADERS_TYPES_H
#define _SHADERS_TYPES_H

#if __cplusplus
#include "Math/Matrix.h"

using uint = uint32_t;

using float2 = Alimer::Vector2;
using float3 = Alimer::Vector3;
using float4 = Alimer::Vector4;

using float3x3 = Alimer::float3x4;
using float4x4 = Alimer::float4x4;

#else

#endif

struct DrawData
{
    float4x4 world;
};

struct CameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
};

struct PushDataConstants
{
    uint dataIndex;
};

#endif // _COMMON_SHADERS_H
