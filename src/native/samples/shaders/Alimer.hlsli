// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_SHADER__
#define _ALIMER_SHADER__

#ifndef __cplusplus
    #define _ALIMER_MERGE_TOKENS(a, b) a##b
    #define ALIMER_MERGE_TOKENS(a, b) _ALIMER_MERGE_TOKENS(a, b)
#endif

// Compiler detection (see: https://github.com/microsoft/DirectXShaderCompiler/wiki/Predefined-Version-Macros)
#ifdef __hlsl_dx_compiler
    #ifdef __spirv__
        #define ALIMER_SPIRV
        #define ALIMER_PRINTF_AVAILABLE
    #else
        #define ALIMER_DXIL
    #endif
#endif

// Shader model
#define ALIMER_SHADER_MODEL (__SHADER_TARGET_MAJOR * 10 + __SHADER_TARGET_MINOR)

#define CONCAT_X(a, b) a##b
#define CONCAT(a, b) CONCAT_X(a, b)

#ifdef __spirv__
#   define ALIMER_PUSH_CONSTANTS(type, name, slot) [[vk::push_constant]] type name
#else
#   define ALIMER_PUSH_CONSTANTS(type, name, slot) ConstantBuffer<type> name : register(ALIMER_MERGE_TOKENS(b, slot), space0)
#endif

/* Static Samplers */
SamplerState SamplerPointClamp : register(s100, space1000);
SamplerState SamplerPointWrap : register(s101, space1000);
SamplerState SamplerPointMirror : register(s102, space1000);
SamplerState SamplerLinearClamp : register(s103, space1000);
SamplerState SamplerLinearWrap : register(s104, space1000);
SamplerState SamplerLinearMirror : register(s105, space1000);
SamplerState SamplerAnisotropicClamp : register(s106, space1000);
SamplerState SamplerAnisotropicWrap : register(s107, space1000);
SamplerState SamplerAnisotropicMirror : register(s108, space1000);
SamplerComparisonState SamplerComparisonDepth : register(s109, space1000);

#ifdef __spirv__
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 1001;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
#else
SamplerState bindlessSamplers[] : register(space1000);
Texture2D bindlessTexture2D[] : register(space1001);
#endif

#endif // _ALIMER_SHADER__
