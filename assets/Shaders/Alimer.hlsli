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
#if defined(METAL)
    #define ALIMER_METAL
#elif defined(__spirv__)
    #define ALIMER_SPIRV
    #define ALIMER_PRINTF_AVAILABLE
#else
    #define ALIMER_DXIL
#endif

#if defined(METAL)
#   define ALIMER_PUSH_CONSTANTS(Type) ConstantBuffer<Type> push : register(b0)
#elif defined(__spirv__)
#   define ALIMER_PUSH_CONSTANTS(Type) [[vk::push_constant]] Type push
#else
#   define ALIMER_PUSH_CONSTANTS(Type) ConstantBuffer<Type> push : register(b999, space0)
#endif

#include "Math.hlsli"

/* Static Samplers */
SamplerState SamplerPointClamp : register(s100);
SamplerState SamplerPointWrap : register(s101);
SamplerState SamplerPointMirror : register(s102);
SamplerState SamplerLinearClamp : register(s103);
SamplerState SamplerLinearWrap : register(s104);
SamplerState SamplerLinearMirror : register(s105);
SamplerState SamplerAnisotropicClamp : register(s106);
SamplerState SamplerAnisotropicWrap : register(s107);
SamplerState SamplerAnisotropicMirror : register(s108);
SamplerComparisonState SamplerComparisonDepth : register(s109);

#endif // _ALIMER_SHADER__
