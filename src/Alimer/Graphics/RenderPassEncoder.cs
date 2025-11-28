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
    private IndexType _indexType;
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
        _indexType = IndexType.Uint16;
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

    public void SetIndexBuffer(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0)
    {
#if VALIDATE_USAGE
        if ((buffer.Usage & BufferUsage.Index) == 0)
        {
            throw new GraphicsException(
                $"Buffer cannot be bound as index buffer because it was not created with BufferUsage.Index.");
        }

        _indexBuffer = buffer;
        _indexType = indexType;
#endif

        SetIndexBufferCore(buffer, indexType, offset);
    }

    public void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        SetViewport(new Viewport(x, y, width, height, minDepth, maxDepth));
    }

    public void SetScissorRect(int width, int height)
    {
        SetScissorRect(new System.Drawing.Rectangle(0, 0, width, height));
    }

    public void SetScissorRect(int x, int y, int width, int height)
    {
        SetScissorRect(new System.Drawing.Rectangle(x, y, width, height));
    }

    public void SetBlendColor(float red, float green, float blue, float alpha)
    {
        SetBlendColor(new Color(red, green, blue, alpha));
    }

    protected abstract void SetPipelineCore(RenderPipeline pipeline);
    public abstract void SetViewport(in Viewport viewport);
    public abstract void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0);
    public abstract void SetScissorRect(in System.Drawing.Rectangle rect);
    public abstract void SetStencilReference(uint reference);
    public abstract void SetBlendColor(in Color color);
    public abstract void SetShadingRate(ShadingRate rate);
    public abstract void SetDepthBounds(float minBounds, float maxBounds);
    protected abstract void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0);
    protected abstract void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0);

    #region Validation
#if VALIDATE_USAGE
    [Conditional("VALIDATE_USAGE")]
    private void ValidateIndexBuffer(uint indexCount)
    {
        if (_indexBuffer == null)
        {
            throw new GraphicsException($"An index buffer must be bound before {nameof(CommandBuffer)}.{nameof(DrawIndexed)} can be called.");
        }

        ulong indexFormatSize = _indexType == IndexType.Uint16 ? 2u : 4u;
        ulong bytesNeeded = indexCount * indexFormatSize;
        if (_indexBuffer.Size < bytesNeeded)
        {
            throw new GraphicsException(
                $"The active index buffer does not contain enough data to satisfy the given draw command. {bytesNeeded} bytes are needed, but the buffer only contains {_indexBuffer.Size}.");
        }
    }
#endif
    #endregion
}
