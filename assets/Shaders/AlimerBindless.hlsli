// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_SHADER_BINDLESS__
#define _ALIMER_SHADER_BINDLESS__

#include "Alimer.hlsli"

#if defined(ALIMER_SPIRV)
/* TODO: Use VK_EXT_mutable_descriptor_type */
/* VkDescriptorType */
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLER = 1;
static const uint DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE = 2;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE = 3;
static const uint DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER = 4;

[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLER)]] SamplerState bindlessSamplers[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2D<float4> bindlessTexture2D[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] Texture2DArray<float4> bindlessTexture2DArray[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_SAMPLED_IMAGE)]] TextureCube<float4> bindlessTextureCube[];
[[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_IMAGE)]] RWTexture2D<float4> bindlessRWTexture2D[];

#define ALIMER_STRUCTURED_BUFFER(type, name) [[vk::binding(0, DESCRIPTOR_SET_BINDLESS_STORAGE_BUFFER)]] StructuredBuffer<type> name[]

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
static const BindlessResource<Texture2D<float4> > bindlessTexture2D;
static const BindlessResource<Texture2DArray<float4> > bindlessTexture2DArray;
static const BindlessResource<TextureCube<float4> > bindlessTextureCube;
static const BindlessResource<RWTexture2D<float4> > bindlessRWTexture2D;

#define ALIMER_STRUCTURED_BUFFER(type, name) static const BindlessResource<StructuredBuffer<type> > name

#else
SamplerState bindlessSamplers[] : register(s0, space4);
Texture2D bindlessTexture2D[] : register(t0, space5);
#endif

#endif // _ALIMER_SHADER_BINDLESS__
