// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
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

    public void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        SetViewport(new Viewport(x, y, width, height, minDepth, maxDepth));
    }

    public void SetScissorRect(int width, int height)
    {
        SetScissorRect(new Rectangle(0, 0, width, height));
    }

    public void SetScissorRect(int x, int y, int width, int height)
    {
        SetScissorRect(new Rectangle(x, y, width, height));
    }

    public void SetBlendColor(float red, float green, float blue, float alpha)
    {
        SetBlendColor(new Numerics.Color(red, green, blue, alpha));
    }

    public abstract void SetViewport(in Viewport viewport);
    public abstract void SetViewports(ReadOnlySpan<Viewport> viewports, int count = 0);
    public abstract void SetScissorRect(in Rectangle rect);
    public abstract void SetStencilReference(uint reference);
    public abstract void SetBlendColor(Numerics.Color color);
    public abstract void SetShadingRate(ShadingRate rate);
    public abstract void SetDepthBounds(float minBounds, float maxBounds);

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
