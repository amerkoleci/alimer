// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Pipeline"/>.
/// </summary>
public readonly record struct RenderPipelineDescription
{
    public RenderPipelineDescription(
        PipelineLayout layout, 
        ShaderStageDescription[] shaderStages, 
        PixelFormat[] colorFormats, 
        PixelFormat depthStencilFormat = PixelFormat.Undefined)
        : this(layout, shaderStages, Array.Empty<VertexBufferLayout>(), colorFormats, depthStencilFormat)
    {
    }

    public RenderPipelineDescription(
        PipelineLayout layout, 
        ShaderStageDescription[] shaderStages, 
        VertexBufferLayout[] vertexBufferLayouts, 
        PixelFormat[] colorFormats,
        PixelFormat depthStencilFormat = PixelFormat.Undefined)
    {
        Guard.IsNotNull(layout, nameof(layout));
        Guard.IsNotNull(shaderStages, nameof(shaderStages));

        Layout = layout;
        ShaderStages = shaderStages;
        VertexBufferLayouts = vertexBufferLayouts;

        BlendState = BlendState.Opaque;
        RasterizerState = RasterizerState.CullBack;
        DepthStencilState = DepthStencilState.DepthDefault;
        PrimitiveTopology = PrimitiveTopology.TriangleList;
        PatchControlPoints = 0;
        ColorFormats = colorFormats;
        DepthStencilFormat = depthStencilFormat;
        SampleCount = TextureSampleCount.Count1;
    }

    public PipelineLayout Layout { get; init; }

    public ShaderStageDescription[] ShaderStages { get; init; }

    public VertexBufferLayout[] VertexBufferLayouts { get; init; }

    public BlendState BlendState { get; init; }

    public RasterizerState RasterizerState { get; init; }

    public DepthStencilState DepthStencilState { get; init; }

    public PrimitiveTopology PrimitiveTopology { get; init; }

    public uint PatchControlPoints { get; init; } = 1u;

    public PixelFormat[] ColorFormats { get; init; }
    public PixelFormat DepthStencilFormat  { get; init; } = PixelFormat.Undefined;

    public TextureSampleCount SampleCount { get; init; } = 0;

    /// <summary>
    /// Gets or sets the label of <see cref="Pipeline"/>.
    /// </summary>
    public string? Label { get; init; }
}
