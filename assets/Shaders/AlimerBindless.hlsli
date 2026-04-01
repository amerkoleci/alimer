// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_SHADER_BINDLESS__
#define _ALIMER_SHADER_BINDLESS__

#include "Alimer.hlsli"

#if defined(ALIMER_SPIRV)
/* TODO: Use VK_EXT_mutable_descriptor_type */
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 2;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 3;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 4;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER = 5;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D<float4> bindlessTexture2D[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTexture2D[];
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
static const BindlessResource<RWTexture2D<float4> > bindlessRWTexture2D;
#else
SamplerState bindlessSamplers[] : register(s0, space4);
Texture2D bindlessTexture2D[] : register(t0, space5);
#endif

#ifdef _SHADER_DEFINITIONS_H_

/* TODO: Handle in common and better way */
#if defined(ALIMER_SPIRV)
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<GPUInstance> bindlessGPUInstance[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<GPUMaterialPBR> bindlessGPUMaterial[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<GPULight> bindlessGPULight[];
#elif ALIMER_SHADER_MODEL >= 66
static const BindlessResource<StructuredBuffer<GPUInstance> > bindlessGPUInstance;
static const BindlessResource<StructuredBuffer<GPUMaterialPBR> > bindlessGPUMaterial;
static const BindlessResource<StructuredBuffer<GPULight> > bindlessGPULight;
#else
#endif
#endif

struct PushConstants
{
    int InstanceBufferIndex;
    int MaterialBufferIndex;
    int LightBufferIndex;
    float _pad;
};
ALIMER_PUSH_CONSTANTS(PushConstants);

// Frame (space 1)
ConstantBuffer<PerFrameData> frame : register(b0, space1);
TextureCube<float4> environmentTexture : register(t0, space1);
SamplerState environmentSampler : register(s0, space1);

// View (space 0)
ConstantBuffer<PerViewData> view : register(b0, space0);

#endif // _ALIMER_SHADER_BINDLESS__
