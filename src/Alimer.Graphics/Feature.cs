// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum Feature
{
    Depth32FloatStencil8,
    TimestampQuery,
    PipelineStatisticsQuery,
    TextureCompressionBC,
    TextureCompressionETC2,
    TextureCompressionASTC,
    TextureCompressionASTC_HDR,
    IndirectFirstInstance,
    ShaderFloat16,
    RG11B10UfloatRenderable,
    BGRA8UnormStorage,

    TessellationShader,
    DepthBoundsTest,
    SamplerClampToBorder,
    SamplerMirrorClampToEdge,
    SamplerMinMax,
    DepthResolveMinMax,
    StencilResolveMinMax,

    ConservativeRasterization,
    CacheCoherentUMA,
    Predication,

    DescriptorIndexing,
    VariableRateShading,
    VariableRateShadingTier2,
    RayTracing,
    RayTracingTier2,
    MeshShader,
}
