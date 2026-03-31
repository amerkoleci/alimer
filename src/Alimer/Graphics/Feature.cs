// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum Feature
{
    PipelineStatisticsQuery,
    TextureCompressionBC,
    TextureCompressionETC2,
    TextureCompressionASTC,
    TextureCompressionASTC_HDR,
    IndirectFirstInstance,
    ShaderFloat16,

    /// <summary>
    /// <see cref="QueryType.Timestamp"/> used on copy command queue
    /// </summary>
    CopyQueueTimestampQuery,
    DepthBoundsTest,
    SamplerClampToBorder,
    SamplerMirrorClampToEdge,
    SamplerMinMax,
    DepthResolveMinMax,
    StencilResolveMinMax,

    ConservativeRasterization,
    CacheCoherentUMA,
    Predication,

    Bindless,
}
