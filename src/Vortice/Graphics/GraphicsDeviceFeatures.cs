// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics;

public readonly struct GraphicsDeviceFeatures
{
    public bool IndependentBlend { get; init; }
    public bool ComputeShader { get; init; }
    public bool TessellationShader { get; init; }
    public bool MultiViewport { get; init; }
    public bool IndexUInt32 { get; init; }
    public bool MultiDrawIndirect { get; init; }
    public bool FillModeNonSolid { get; init; }
    public bool SamplerAnisotropy { get; init; }
    public bool TextureCompressionETC2 { get; init; }
    public bool TextureCompressionASTC_LDR { get; init; }
    public bool TextureCompressionBC { get; init; }
    public bool TextureCubeArray { get; init; }
    public bool Raytracing { get; init; }
}
