// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract class RenderContext : ComputeContext
{
    protected ShadingRate _currentShadingRate = ShadingRate.Invalid;

    protected RenderContext()
    {
    }

    /// <inheritdoc />
    public override QueueType QueueType => QueueType.Graphics;
    
    public abstract Texture? AcquireSwapChainTexture(SwapChain swapChain);

    public void BeginRenderPass(in RenderPassDescription renderPass)
    {
        Guard.IsFalse(_insideRenderPass);

        BeginRenderPassCore(in renderPass);
        _insideRenderPass = true;
    }

    public void EndRenderPass()
    {
        Guard.IsTrue(_insideRenderPass);

        EndRenderPassCore();
        _insideRenderPass = false;
    }

    public IDisposable PushScopedPassPass(in RenderPassDescription renderPass)
    {
        BeginRenderPass(renderPass);
        return new ScopedRenderPass(this);
    }


    protected override void Reset(uint frameIndex)
    {
        base.Reset(frameIndex);
        _currentShadingRate = ShadingRate.Invalid;
        //frameAllocators[frameIndex].Reset();
    }

    protected abstract void BeginRenderPassCore(in RenderPassDescription renderPass);
    protected abstract void EndRenderPassCore();

    #region Nested
    readonly struct ScopedRenderPass : IDisposable
    {
        private readonly RenderContext _context;

        public ScopedRenderPass(RenderContext context)
        {
            _context = context;
        }

        public void Dispose()
        {
            _context.EndRenderPass();
        }
    }
    #endregion
}
