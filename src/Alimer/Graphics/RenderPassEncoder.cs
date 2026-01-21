// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract unsafe class RenderPassEncoder : CommandEncoder
{
    protected ShadingRate _currentShadingRate = ShadingRate.Invalid;
#if VALIDATE_USAGE
    private GraphicsBuffer? _indexBuffer;
    private IndexFormat _indexFormat;
#endif

    protected RenderPassEncoder()
    {
    }

    protected virtual void Reset()
    {
        _hasLabel = false;
        _currentShadingRate = ShadingRate.Invalid;

#if VALIDATE_USAGE
        _indexBuffer = default;
        _indexFormat = IndexFormat.UInt16;
#endif
    }

    public void SetPipeline(RenderPipeline pipeline)
    {
        Guard.IsNotNull(pipeline, nameof(pipeline));

        SetPipelineCore(pipeline);
    }

    public void SetVertexBuffer(uint slot, GraphicsBuffer buffer, ulong offset = 0)
    {
#if VALIDATE_USAGE
        if ((buffer.Usage & BufferUsage.Vertex) == 0)
        {
            throw new GraphicsException(
                $"Buffer cannot be bound as Vertex buffer because it was not created with BufferUsage.Vertex.");
        }
#endif

        SetVertexBufferCore(slot, buffer, offset);
    }

    public void SetIndexBuffer(GraphicsBuffer buffer, IndexFormat format, ulong offset = 0)
    {
#if VALIDATE_USAGE
        if ((buffer.Usage & BufferUsage.Index) == 0)
        {
            throw new GraphicsException(
                $"Buffer cannot be bound as index buffer because it was not created with BufferUsage.Index.");
        }

        _indexBuffer = buffer;
        _indexFormat = format;
#endif

        SetIndexBufferCore(buffer, format, offset);
    }

    public void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        SetViewport(new Viewport(x, y, width, height, minDepth, maxDepth));
    }

    public void SetScissorRect(int width, int height)
    {
        SetScissorRect(new RectI(width, height));
    }

    public void SetScissorRect(int x, int y, int width, int height)
    {
        SetScissorRect(new RectI(x, y, width, height));
    }

    public void SetBlendColor(float red, float green, float blue, float alpha)
    {
        SetBlendColor(new Color(red, green, blue, alpha));
    }

    /// <summary>
    /// Draw non-indexed geometry.
    /// </summary>
    /// <param name="vertexCount"></param>
    /// <param name="instanceCount"></param>
    /// <param name="firstVertex"></param>
    /// <param name="firstInstance"></param>
    public void Draw(uint vertexCount, uint instanceCount = 1, uint firstVertex = 0, uint firstInstance = 0)
    {
        DrawCore(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    /// <summary>
    /// Draw indexed geometry.
    /// </summary>
    /// <param name="indexCount"></param>
    /// <param name="instanceCount"></param>
    /// <param name="firstIndex"></param>
    /// <param name="baseVertex"></param>
    /// <param name="firstInstance"></param>
    public void DrawIndexed(uint indexCount, uint instanceCount = 1, uint firstIndex = 0, int baseVertex = 0, uint firstInstance = 0)
    {
#if VALIDATE_USAGE
        ValidateIndexBuffer(indexCount);
#endif

        DrawIndexedCore(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    /// <summary>
    /// Draw primitives with indirect parameters
    /// </summary>
    /// <param name="indirectBuffer"></param>
    /// <param name="indirectBufferOffset"></param>
    public void DrawIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DrawIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void DrawIndirectCount(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount = 1)
    {
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);
        ValidateIndirectBuffer(countBuffer);
        ValidateIndirectOffset(countBufferOffset);

        DrawIndirectCountCore(indirectBuffer, indirectBufferOffset, countBuffer, countBufferOffset, maxCount);
    }

    /// <summary>
    /// Draw primitives with indirect parameters and indexed vertices
    /// </summary>
    /// <param name="indirectBuffer"></param>
    /// <param name="indirectBufferOffset"></param>
    public void DrawIndexedIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);

        DrawIndexedIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void DrawIndexedIndirectCount(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount = 1)
    {
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);
        CommandEncoder.ValidateIndirectBuffer(countBuffer);
        CommandEncoder.ValidateIndirectOffset(countBufferOffset);

        DrawIndexedIndirectCountCore(indirectBuffer, indirectBufferOffset, countBuffer, countBufferOffset, maxCount);
    }

    public void DispatchMesh(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        PreDispatchMeshValidation();

        DispatchMeshCore(groupCountX, groupCountY, groupCountZ);
    }

    public void DispatchMeshIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        PreDispatchMeshValidation();
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DispatchMeshIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void DispatchMeshIndirectCount(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount = 1)
    {
        PreDispatchMeshValidation();
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);
        ValidateIndirectBuffer(countBuffer);
        ValidateIndirectOffset(countBufferOffset);

        DispatchMeshIndirectCountCore(indirectBuffer, indirectBufferOffset, countBuffer, countBufferOffset, maxCount);
    }


    protected abstract void SetPipelineCore(RenderPipeline pipeline);
    public abstract void SetViewport(in Viewport viewport);
    public abstract void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0);
    public abstract void SetScissorRect(in RectI rect);
    public abstract void SetStencilReference(uint reference);
    public abstract void SetBlendColor(in Color color);
    public abstract void SetShadingRate(ShadingRate rate);

    public void SetDepthBounds(float minBounds, float maxBounds)
    {
        if (!Device.QueryFeatureSupport(Feature.DepthBoundsTest))
        {
            throw new GraphicsException($"{nameof(Feature.DepthBoundsTest)} feature is not supported");
        }

        SetDepthBoundsCore(minBounds, maxBounds);
    }

    protected abstract void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0);
    protected abstract void SetIndexBufferCore(GraphicsBuffer buffer, IndexFormat format, ulong offset = 0);
    protected abstract void SetDepthBoundsCore(float minBounds, float maxBounds);

    protected abstract void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance);
    protected abstract void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance);
    protected abstract void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);
    protected abstract void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);
    protected abstract void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);

    #region Validation
#if VALIDATE_USAGE
    [Conditional("VALIDATE_USAGE")]
    private void ValidateIndexBuffer(uint indexCount)
    {
        if (_indexBuffer == null)
        {
            throw new GraphicsException($"An index buffer must be bound before {nameof(CommandBuffer)}.{nameof(DrawIndexed)} can be called.");
        }

        ulong indexFormatSize = _indexFormat == IndexFormat.UInt16 ? 2u : 4u;
        ulong bytesNeeded = indexCount * indexFormatSize;
        if (_indexBuffer.Size < bytesNeeded)
        {
            throw new GraphicsException(
                $"The active index buffer does not contain enough data to satisfy the given draw command. {bytesNeeded} bytes are needed, but the buffer only contains {_indexBuffer.Size}.");
        }
    }
#endif


    [Conditional("VALIDATE_USAGE")]
    private void PreDispatchMeshValidation()
    {
        if (!Device.QueryFeatureSupport(Feature.MeshShader))
        {
            throw new GraphicsException($"Device doesn't support Feature.MeshShader.");
        }
    }
    #endregion
}
