// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract class CommandBuffer
{
    protected bool _hasLabel;
    protected bool _insideRenderPass;
    protected ShadingRate _currentShadingRate = ShadingRate.Invalid;

    protected CommandBuffer(GraphicsDevice device)
    {
        Guard.IsNotNull(device, nameof(device));

        Device = device;
    }

    /// <summary>
    /// Get the <see cref="CommandQueue"/> object that will execute this CommandBuffer.
    /// </summary>
    public abstract CommandQueue Queue { get; }

    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public GraphicsDevice Device { get; }

    public abstract void Commit();

    public abstract void PushDebugGroup(string groupLabel);
    public abstract void PopDebugGroup();
    public abstract void InsertDebugMarker(string debugLabel);

    public abstract Texture? AcquireSwapChainTexture(SwapChain swapChain);

    public IDisposable PushScopedDebugGroup(string groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
    }

    protected void Reset(uint frameIndex)
    {
        _hasLabel = false;
        _insideRenderPass = false;
        _currentShadingRate = ShadingRate.Invalid;
        //frameAllocators[frameIndex].Reset();
    }

    readonly struct ScopedDebugGroup : IDisposable
    {
        private readonly CommandBuffer _commandBuffer;

        public ScopedDebugGroup(CommandBuffer commandBuffer)
        {
            _commandBuffer = commandBuffer;
        }

        public void Dispose() => _commandBuffer.PopDebugGroup();
    }
}
