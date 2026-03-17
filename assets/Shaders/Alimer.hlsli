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
    #if defined(METAL)
        #define ALIMER_METAL
    #elif defined(__spirv__)
        #define ALIMER_SPIRV
        #define ALIMER_PRINTF_AVAILABLE
    #else
        #define ALIMER_DXIL
    #endif
#endif

#ifdef __spirv__
#   define ALIMER_PUSH_CONSTANTS(type, name) [[vk::push_constant]] type name
#else
#   define ALIMER_PUSH_CONSTANTS(type, name) ConstantBuffer<type> name : register(b999, space0)
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

//#ifndef ALIMER_BINDLESS
//#define ALIMER_BINDLESS 1
//#endif

#if defined(ALIMER_BINDLESS)
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 1001;
#ifdef __spirv__
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 1002;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#else
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#endif
#endif /* defined(ALIMER_BINDLESS)  */

#endif // _ALIMER_SHADER__
