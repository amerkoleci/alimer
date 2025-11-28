// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;

namespace Alimer.Graphics;

public abstract class CommandBuffer
{
    protected bool _encoderActive;
    protected bool _hasLabel;


    protected CommandBuffer()
    {
    }

    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public abstract GraphicsDevice Device { get; }

    protected virtual void Reset(uint frameIndex)
    {
        _hasLabel = false;
        _encoderActive = false;
        //_currentShadingRate = ShadingRate.Invalid;
        //frameAllocators[frameIndex].Reset();

    }

    /// <summary>
    /// Flush existing commands to the GPU and optinally wait for completion.
    /// </summary>
    /// <param name="waitForCompletion"></param>
    public abstract void Flush(bool waitForCompletion = false);

    public abstract void PushDebugGroup(Utf8ReadOnlyString groupLabel);
    public abstract void PopDebugGroup();
    public abstract void InsertDebugMarker(Utf8ReadOnlyString debugLabel);

    public IDisposable PushScopedDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
    }

    public ComputePassEncoder BeginComputePass() => BeginComputePass(new ComputePassDescriptor());

    public ComputePassEncoder BeginComputePass(in ComputePassDescriptor descriptor)
    {
        if (_encoderActive)
        {
            throw new GraphicsException("CommandEncoder already active");
        }

        ComputePassEncoder computePassEncoder = BeginComputePassCore(in descriptor);
        _encoderActive = true;
        return computePassEncoder;
    }

    public RenderPassEncoder BeginRenderPass(in RenderPassDescriptor renderPass)
    {
        if (_encoderActive)
        {
            throw new GraphicsException("CommandEncoder already active");
        }

        RenderPassEncoder renderPassEncoder = BeginRenderPassCore(in renderPass);
        _encoderActive = true;
        return renderPassEncoder;
    }

    protected abstract ComputePassEncoder BeginComputePassCore(in ComputePassDescriptor descriptor);
    protected abstract RenderPassEncoder BeginRenderPassCore(in RenderPassDescriptor descriptor);


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
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);

        DrawIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void DrawIndirectCount(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount = 1)
    {
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);
        CommandEncoder.ValidateIndirectBuffer(countBuffer);
        CommandEncoder.ValidateIndirectOffset(countBufferOffset);

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
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);

        DispatchMeshIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    public void DispatchMeshIndirectCount(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount = 1)
    {
        PreDispatchMeshValidation();
        CommandEncoder.ValidateIndirectBuffer(indirectBuffer);
        CommandEncoder.ValidateIndirectOffset(indirectBufferOffset);
        CommandEncoder.ValidateIndirectBuffer(countBuffer);
        CommandEncoder.ValidateIndirectOffset(countBufferOffset);

        DispatchMeshIndirectCountCore(indirectBuffer, indirectBufferOffset, countBuffer, countBufferOffset, maxCount);
    }

    public abstract void Present(SwapChain swapChain);

    protected abstract void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance);
    protected abstract void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance);
    protected abstract void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);
    protected abstract void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);

    protected abstract void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
    protected abstract void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount);

    [Conditional("VALIDATE_USAGE")]
    private void PreDispatchMeshValidation()
    {
        if (!Device.Adapter.QueryFeatureSupport(Feature.MeshShader))
        {
            throw new GraphicsException($"Device doesn't support Feature.MeshShader.");
        }
    }

    #region Nested
    readonly struct ScopedDebugGroup(CommandBuffer commandBuffer) : IDisposable
    {
        private readonly CommandBuffer _commandBuffer = commandBuffer;

        public void Dispose() => _commandBuffer.PopDebugGroup();
    }
    #endregion
}
