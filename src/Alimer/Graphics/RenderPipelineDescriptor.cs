// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics.CodeAnalysis;

namespace Alimer.Graphics;

/// <summary>
/// Structure that describes the <see cref="RenderPipeline"/>.
/// </summary>
public ref struct RenderPipelineDescriptor
{
    /// <summary>
    /// Gets or sets the label of <see cref="RenderPipeline"/>.
    /// </summary>
    public string? Label;

    public ShaderModule? VertexShader;
    public ShaderModule? FragmentShader;

    public ShaderModule? MeshShader;
    public ShaderModule? AmplificationShader;

    public Span<VertexBufferLayout> VertexBufferLayouts;

    public BlendState BlendState;

    public RasterizerState RasterizerState;

    public DepthStencilState DepthStencilState;

    public PrimitiveTopology PrimitiveTopology;

    /// <summary>
    /// Index format for strip topologies (when using primitive restart -> <see cref="PrimitiveTopology.LineStrip"/> or <see cref="PrimitiveTopology.TriangleStrip"/>).
    /// </summary>
	public IndexFormat StripIndexFormat = IndexFormat.Undefined;

    public Span<PixelFormat> ColorFormats;
    public PixelFormat DepthStencilFormat = PixelFormat.Undefined;

    public TextureSampleCount SampleCount = TextureSampleCount.Count1;

    public RenderPipelineDescriptor(
        Span<PixelFormat> colorFormats,
        PixelFormat depthStencilFormat = PixelFormat.Undefined)
        : this([], colorFormats, depthStencilFormat)
    {
    }

    public RenderPipelineDescriptor(
        Span<VertexBufferLayout> vertexBufferLayouts,
        Span<PixelFormat> colorFormats,
        PixelFormat depthStencilFormat = PixelFormat.Undefined)
    {
        VertexBufferLayouts = vertexBufferLayouts;

        BlendState = BlendState.Opaque;
        RasterizerState = RasterizerState.CullBack;
        DepthStencilState = DepthStencilState.DepthDefault;
        PrimitiveTopology = PrimitiveTopology.TriangleList;
        ColorFormats = colorFormats;
        DepthStencilFormat = depthStencilFormat;
        SampleCount = TextureSampleCount.Count1;
    }
}
