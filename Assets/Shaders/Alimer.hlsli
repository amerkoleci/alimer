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

#if defined(ALIMER_METAL)
#   define ALIMER_PUSH_CONSTANTS(type) ConstantBuffer<type> push : register(b0)
#elif defined(ALIMER_SPIRV)
#   define ALIMER_PUSH_CONSTANTS(type) [[vk::push_constant]] ConstantBuffer<type> push
#else
#   define ALIMER_PUSH_CONSTANTS(type) ConstantBuffer<type> push : register(b999, space0)
#endif

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

#if defined(ALIMER_SPIRV)
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 1;
//static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 2;
//static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1000;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D bindlessTexture2D[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2DArray bindlessTexture2DArray[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] TextureCube bindlessTextureCube[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] TextureCube<half4> bindlessTextureCube_half4[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] TextureCubeArray bindlessTextureCubeArray[];

//[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#elif ALIMER_SHADER_MODEL >= 66
template<typename T>
struct BindlessResource
{
	T operator[](uint index) { return (T)ResourceDescriptorHeap[index]; }
};
template<>
struct BindlessResource<SamplerState>
{
	SamplerState operator[](uint index) { return (SamplerState)SamplerDescriptorHeap[index]; }
};
static const BindlessResource<SamplerState> bindlessSamplers;
static const BindlessResource<Texture2D> bindlessTexture2D;
#else
SamplerState    bindlessSamplers[]      : register(space1000);
Texture2D       bindlessTexture2D[]     : register(space1001);
//ByteAddressBuffer bindlessBuffers[] : register(space1);
#endif

#endif // _ALIMER_SHADER__
