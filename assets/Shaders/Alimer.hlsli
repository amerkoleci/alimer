// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_SHADER__
#define _ALIMER_SHADER__

#ifndef __cplusplus
    #define _ALIMER_MERGE_TOKENS(a, b) a##b
    #define ALIMER_MERGE_TOKENS(a, b) _ALIMER_MERGE_TOKENS(a, b)
#endif

// Shader model
#define ALIMER_SHADER_MODEL (__SHADER_TARGET_MAJOR * 10 + __SHADER_TARGET_MINOR)

// Compiler detection (see: https://github.com/microsoft/DirectXShaderCompiler/wiki/Predefined-Version-Macros)
#ifdef __hlsl_dx_compiler
    #ifdef __spirv__
        #define ALIMER_SPIRV
        #define ALIMER_PRINTF_AVAILABLE
    #else
        #define ALIMER_DXIL
    #endif
#endif

#ifdef __spirv__
#   define ALIMER_PUSH_CONSTANTS(type, name) [[vk::push_constant]] type name
#   define ALIMER_VERTEX_ATTRIBUTE(type, name, loc) [[vk::location(loc)]] type name : ALIMER_MERGE_TOKENS(ATTRIBUTE, loc)
#else
#   define ALIMER_PUSH_CONSTANTS(type, name) ConstantBuffer<type> name : register(b999, space0)
#   define ALIMER_VERTEX_ATTRIBUTE(type, name, loc) type name : ALIMER_MERGE_TOKENS(ATTRIBUTE, loc)
#endif

#include "Math.hlsli"

/* Static Samplers */
SamplerState SamplerPointClamp                : register(s100);
SamplerState SamplerPointWrap                 : register(s101);
SamplerState SamplerPointMirror               : register(s102);
SamplerState SamplerLinearClamp               : register(s103);
SamplerState SamplerLinearWrap                : register(s104);
SamplerState SamplerLinearMirror              : register(s105);
SamplerState SamplerAnisotropicClamp          : register(s106);
SamplerState SamplerAnisotropicWrap           : register(s107);
SamplerState SamplerAnisotropicMirror         : register(s108);
SamplerComparisonState SamplerComparisonDepth : register(s109);

/* Bindless */
// TODO: Allow engine side static sampler configuration and remove definitions in code

#ifdef __spirv__
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 1001;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 1002;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#else
SamplerState    bindlessSamplers[]      : register(space1000);
Texture2D       bindlessTexture2D[]     : register(space1001);
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#endif

#endif // _ALIMER_SHADER__
