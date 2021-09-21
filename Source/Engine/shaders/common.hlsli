// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _COMMON_SHADERS_H
#define _COMMON_SHADERS_H

#include <alimer/shaders/types.h>

#ifdef SPIRV

#define PUSH_CONSTANT(type, name) [[vk::push_constant]] type name;

#else /* DXIL */

#define PUSH_CONSTANT(type, name) ConstantBuffer<type> name : register(b999)

#endif

/* Samplers */
SamplerState PointWrapSampler           : register(s0);
SamplerState PointClampSampler          : register(s1);
SamplerState LinearWrapSampler          : register(s2);
SamplerState LinearClampSampler         : register(s3);
SamplerState AnisotropicWrapSampler     : register(s4);
SamplerState AnisotropicClampSampler    : register(s5);

// Bindless descriptors
Texture2D Texture2DTable[] : register(t0, space1);
//ByteAddressBuffer SRVBuffers[] : register(t0, space1);
//Texture2DArray Tex2DArrayTable[] : register(space2);

#endif // _COMMON_SHADERS_H
