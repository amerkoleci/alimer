// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

public enum Feature
{
	DepthClipControl = 0,
    Depth32FloatStencil8 = 1,
    TimestampQuery = 2,
    PipelineStatisticsQuery = 3,
    TextureCompressionBC = 4,
    TextureCompressionETC2 = 5,
    TextureCompressionASTC = 6,
    IndirectFirstInstance = 7,
    ShaderFloat16 = 8,
    GeometryShader = 9,
    TessellationShader = 10,
    DepthBoundsTest = 11,
    SamplerAnisotropy = 12,
    SamplerMinMax = 13,
    DescriptorIndexing = 14,
    Predication = 15,
    ShaderOutputViewportIndex = 16,
    VariableRateShading = 17,
    VariableRateShadingTier2 = 18,
    RayTracing = 19,
    RayTracingTier2 = 20,
    MeshShader = 21,
}
