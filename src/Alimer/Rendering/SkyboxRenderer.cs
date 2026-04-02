// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed class SkyboxRenderer : IDisposable
{
    private readonly GpuBuffer _vertexBuffer;
    private readonly GpuBuffer _indexBuffer;
    private readonly PipelineLayout _pipelineLayout;
    private readonly RenderPipeline _renderPipeline;

    public SkyboxRenderer(RenderSystem renderSystem)
    {
        RenderSystem = renderSystem;

        ShaderModule vertexShader = renderSystem.ShaderSystem.GetShaderModule("Skybox", ShaderStages.Vertex);
        ShaderModule fragmentShader = renderSystem.ShaderSystem.GetShaderModule("Skybox", ShaderStages.Fragment);

        Span<float> SKYBOX_VERTS = [
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
        ];

        Span<ushort> SKYBOX_INDICES = [
            0, 1, 2, 2, 3, 0,   // Front
            1, 4, 7, 7, 2, 1,   // Right
            4, 5, 6, 6, 7, 4,   // Back
            5, 0, 3, 3, 6, 5,   // Left
            5, 4, 1, 1, 0, 5,   // Top
            3, 2, 7, 7, 6, 3    // Bottom
        ];

        _vertexBuffer = renderSystem.Device.CreateBuffer(SKYBOX_VERTS, GpuBufferUsage.Vertex);
        _indexBuffer = renderSystem.Device.CreateBuffer(SKYBOX_INDICES, GpuBufferUsage.Index);

        Span <BindGroupLayout> bindGroupLayouts = [renderSystem.ViewBindGroupLayout, renderSystem.FrameBindGroupLayout];

        PipelineLayoutDescriptor descriptor = new(bindGroupLayouts);
        _pipelineLayout = renderSystem.Device.CreatePipelineLayout(in descriptor);

        VertexBufferLayout gpuLayout = new(12, [new VertexAttribute(VertexAttributeSemantic.Position, VertexAttributeFormat.Float32x3)]);
        Span<VertexBufferLayout> geometryLayout = [gpuLayout];

        RenderPipelineDescriptor renderPipelineDesc = new(_pipelineLayout,
            geometryLayout,
            [RenderSystem.ColorFormat], RenderSystem.DepthStencilFormat)
        {
            Label = "RenderPipeline",
            VertexShader = vertexShader,
            FragmentShader = fragmentShader,
            DepthStencilState = DepthStencilState.DepthRead
        };
        _renderPipeline = renderSystem.Device.CreateRenderPipeline(in renderPipelineDesc);
    }

    public RenderSystem RenderSystem { get; }

    public void Dispose()
    {
        _pipelineLayout.Dispose();
        _renderPipeline.Dispose();
        _vertexBuffer.Dispose();
        _indexBuffer.Dispose();
    }

    public void Draw(RenderPassEncoder renderPass)
    {
        // Skybox is part of the frame bind group, which should already be bound prior to calling this method.
        renderPass.SetPipeline(_renderPipeline);
        renderPass.SetVertexBuffer(0, _vertexBuffer);
        renderPass.SetIndexBuffer(_indexBuffer, IndexFormat.Uint16);
        renderPass.DrawIndexed(36); // SKYBOX_INDICES.Length
    }
}
