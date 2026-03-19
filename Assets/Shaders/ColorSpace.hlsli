// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_COLOR_SPACE__
#define _ALIMER_COLOR_SPACE__

float LinearToSrgb(float value)
{
    if (value < 0.00313067)
        return value * 12.92;

    return pow(value, (1.0 / 2.4)) * 1.055 - 0.055;
}

float3 LinearToSrgb(float3 value)
{
    return float3(LinearToSrgb(value.r), LinearToSrgb(value.g), LinearToSrgb(value.b));
}

float3 SRGBToLinear(float3 srgb)
{
    return srgb * (srgb * (srgb * 0.305306011 + 0.682171111) + 0.012522878);
}

float4 SRGBToLinear(float4 color)
{
    return float4(SRGBToLinear(color.rgb), color.a);
}

#endif // _ALIMER_COLOR_SPACE__
