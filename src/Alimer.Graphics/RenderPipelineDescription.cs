// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Pipeline"/>.
/// </summary>
public readonly record struct RenderPipelineDescription
{
    public RenderPipelineDescription(PipelineLayout layout, ShaderStageDescription[] shaderStages)
    {
        Guard.IsNotNull(layout, nameof(layout));
        Guard.IsNotNull(shaderStages, nameof(shaderStages));

        Layout = layout;
        ShaderStages = shaderStages;

        BlendState = BlendState.Opaque;
        RasterizerState = RasterizerState.CullBack;
        DepthStencilState = DepthStencilState.DepthDefault;
        PrimitiveTopology = PrimitiveTopology.TriangleList;
        PatchControlPoints = 0;
        SampleCount = TextureSampleCount.Count1;
    }

    public PipelineLayout Layout { get; init; }

    public ShaderStageDescription[] ShaderStages { get; init; }

    public BlendState BlendState { get; init; }

    public RasterizerState RasterizerState { get; init; }

    public DepthStencilState DepthStencilState { get; init; }
    //public VertexDescriptor VertexDescriptor { get; init; }

    public PrimitiveTopology PrimitiveTopology { get; init; }

    public int PatchControlPoints { get; init; } = 0;

    public TextureSampleCount SampleCount { get; init; } = 0;

    /// <summary>
    /// Gets or sets the label of <see cref="Pipeline"/>.
    /// </summary>
    public string? Label { get; init; }
}
