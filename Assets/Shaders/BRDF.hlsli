// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_BRDF__
#define _ALIMER_BRDF__

#include "Math.hlsli"

float3 FresnelSchlick(in float LdotH, in float3 F0)
{
    return F0 + (1.f - F0) * pow(1.f - LdotH, 5.f);
}

float3 FresnelSchlickRoughness(float LdotH, float3 F0, float roughness)
{
    float v = 1.0f - roughness;
    return F0 + (max(float3(v, v, v), F0) - F0) * pow(1.f - LdotH, 5.f);
}

float DistributionGGX(in float3 N, in float3 H, in float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.f) + 1.f);
    return a2 / max(denom * denom * Pi, 0.001f);
}

float GeometrySchlickGGX(in float NdotV, in float roughness)
{
    const float r = roughness + 1;
    const float k = (r * r) / 8;

    const float num = NdotV;
    const float denom = NdotV * (1 - k) + k;

    return num / denom;
}

float GeometrySmith(in float3 N, in float3 V, in float3 L, in float roughness)
{
    const float NdotV = max(dot(N, V), 0);
    const float NdotL = max(dot(N, L), 0);
    const float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    const float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// From https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
float2 envBRDFApprox(in float roughness, in float NdotV)
{
    float4 c0 = float4(-1.f, -0.0275f, -0.572f, 0.022f);
    float4 c1 = float4(1.f, 0.0425f, 1.04f, -0.04f);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-9.28f * NdotV)) * r.x + r.y;
    return float2(-1.04f, 1.04f) * a004 + r.zw;
}

#endif // _ALIMER_BRDF__
