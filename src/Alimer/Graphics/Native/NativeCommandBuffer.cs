// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using Alimer.Utilities;
using static Alimer.AlimerApi;
namespace Alimer.Graphics.Native;

internal unsafe class NativeCommandBuffer : RenderContext
{
    private readonly NativeCommandQueue _queue;

    public NativeCommandBuffer(NativeCommandQueue queue)
    {
        _queue = queue;
    }

    public override GraphicsDevice Device => _queue.Device;

    public override void Flush(bool waitForCompletion = false) => throw new NotImplementedException();
    public override void InsertDebugMarker(string debugLabel) => throw new NotImplementedException();
    public override void PopDebugGroup() => throw new NotImplementedException();
    public override void Present(SwapChain swapChain) => throw new NotImplementedException();
    public override void PushDebugGroup(Utf8ReadOnlyString groupLabel) => throw new NotImplementedException();
    public override void SetBlendColor(in Numerics.Color color) => throw new NotImplementedException();
    public override void SetDepthBounds(float minBounds, float maxBounds) => throw new NotImplementedException();
    public override void SetScissorRect(in Rectangle rect) => throw new NotImplementedException();
    public override void SetShadingRate(ShadingRate rate) => throw new NotImplementedException();
    public override void SetStencilReference(uint reference) => throw new NotImplementedException();
    public override void SetViewport(in Viewport viewport) => throw new NotImplementedException();
    public override void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0) => throw new NotImplementedException();
    protected override void BeginRenderPassCore(in RenderPassDescription renderPass) => throw new NotImplementedException();
    protected override void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ) => throw new NotImplementedException();
    protected override void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset) => throw new NotImplementedException();
    protected override void DispatchMeshCore(uint groupCountX, uint groupCountY, uint groupCountZ) => throw new NotImplementedException();
    protected override void DispatchMeshIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset) => throw new NotImplementedException();
    protected override void DispatchMeshIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount) => throw new NotImplementedException();
    protected override void DrawCore(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance) => throw new NotImplementedException();
    protected override void DrawIndexedCore(uint indexCount, uint instanceCount, uint firstIndex, int baseVertex, uint firstInstance) => throw new NotImplementedException();
    protected override void DrawIndexedIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset) => throw new NotImplementedException();
    protected override void DrawIndexedIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount) => throw new NotImplementedException();
    protected override void DrawIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset) => throw new NotImplementedException();
    protected override void DrawIndirectCountCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset, GraphicsBuffer countBuffer, ulong countBufferOffset, uint maxCount) => throw new NotImplementedException();
    protected override void EndRenderPassCore() => throw new NotImplementedException();
    protected override void SetBindGroupCore(uint groupIndex, BindGroup bindGroup) => throw new NotImplementedException();
    protected override void SetIndexBufferCore(GraphicsBuffer buffer, IndexType indexType, ulong offset = 0) => throw new NotImplementedException();
    protected override void SetPipelineCore(Pipeline pipeline) => throw new NotImplementedException();
    protected override unsafe void SetPushConstantsCore(uint pushConstantIndex, void* data, uint size) => throw new NotImplementedException();
    protected override void SetVertexBufferCore(uint slot, GraphicsBuffer buffer, ulong offset = 0) => throw new NotImplementedException();
}
