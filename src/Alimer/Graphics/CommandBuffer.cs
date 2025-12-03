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

    public abstract void Present(SwapChain swapChain);

    #region Nested
    readonly struct ScopedDebugGroup(CommandBuffer commandBuffer) : IDisposable
    {
        private readonly CommandBuffer _commandBuffer = commandBuffer;

        public void Dispose() => _commandBuffer.PopDebugGroup();
    }
    #endregion
}
