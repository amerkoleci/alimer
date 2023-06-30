// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="Pipeline"/>.
/// </summary>
public readonly record struct RenderPipelineDescription
{
    public RenderPipelineDescription()
    {
        BlendState = BlendState.Opaque;
        RasterizerState = RasterizerState.CullBack;
        DepthStencilState = DepthStencilState.DepthDefault;
        PrimitiveTopology = PrimitiveTopology.TriangleList;
    }

    public ReadOnlyMemory<byte> VertexShader { get; init; }

    public ReadOnlyMemory<byte> FragmentShader { get; init; }

    public BlendState BlendState { get; init; }

    public RasterizerState RasterizerState { get; init; }

    public DepthStencilState DepthStencilState { get; init; }
    //public VertexDescriptor VertexDescriptor { get; init; }

    public PrimitiveTopology PrimitiveTopology { get; init; }

    /// <summary>
    /// Gets or sets the label of <see cref="Pipeline"/>.
    /// </summary>
    public string? Label { get; init; }
}
