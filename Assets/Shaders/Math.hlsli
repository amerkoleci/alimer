// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_MATH__
#define _ALIMER_MATH__

static const float Pi = 3.141592654f;
static const float TwoPi = 6.283185307f;
static const float PiOver2 = 1.570796327f;
static const float PiOver4 = 0.7853981635f;
static const float InvPi = 0.318309886f;
static const float InvPi2 = 0.159154943f;

inline half4 unpack_half4(in uint2 value)
{
    half4 retVal;
    retVal.x = (half) f16tof32(value.x);
    retVal.y = (half) f16tof32(value.x >> 16u);
    retVal.z = (half) f16tof32(value.y);
    retVal.w = (half) f16tof32(value.y >> 16u);
    return retVal;
}

#endif // _ALIMER_MATH__
