// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum Feature
{
    DepthClipControl,
    Depth32FloatStencil8,
    TimestampQuery,
    PipelineStatisticsQuery,
    TextureCompressionBC,
    TextureCompressionETC2,
    TextureCompressionASTC,
    IndirectFirstInstance,
    ShaderFloat16,
    RG11B10UfloatRenderable,
    BGRA8UnormStorage,
    TessellationShader,
    DepthBoundsTest,
    SamplerAnisotropy,
    SamplerMinMax,
    DepthResolveMinMax,
    StencilResolveMinMax,
    Predication,

    DescriptorIndexing,
    VariableRateShading,
    VariableRateShadingTier2,
    RayTracing,
    RayTracingTier2,
    MeshShader,
}
