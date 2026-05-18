// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef _ALIMER_SHADER_SCENE__
#define _ALIMER_SHADER_SCENE__

#include "AlimerBindless.hlsli"
#include "ShaderTypes.h"

// Frame data
ConstantBuffer<GPUFrameData> frame : register(b0);

/* Push constants for draw calls */
struct ScenePushConstants
{
    int InstanceBufferIndex;
    int MaterialBufferIndex;
    int LightBufferIndex;
    float _pad;
};
ALIMER_PUSH_CONSTANTS(ScenePushConstants);

/* TODO: Handle in common and better way */
ALIMER_STRUCTURED_BUFFER(GPUInstance, bindlessGPUInstance);
ALIMER_STRUCTURED_BUFFER(GPUMaterialPBR, bindlessGPUMaterial);
ALIMER_STRUCTURED_BUFFER(GPULight, bindlessGPULight);

#endif // _ALIMER_SHADER_SCENE__
