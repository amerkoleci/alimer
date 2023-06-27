// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics.VGPU;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.VGPU.VGPU;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics;

public readonly unsafe struct CommandBuffer
{
    /// <summary>
    /// Get the <see cref="GraphicsDevice"/> object that created this object.
    /// </summary>
    public GraphicsDevice Device { get; }

    internal VGPUCommandBuffer Handle { get; }

    internal CommandBuffer(GraphicsDevice device, VGPUCommandBuffer handle)
    {
        Device = device;
        Handle = handle;
    }

    public  void PushDebugGroup(string groupLabel)
    {
        fixed (sbyte* pLabel = groupLabel.GetUtf8Span())
        {
            vgpuPushDebugGroup(Handle, pLabel);
        }
    }

    public  void PopDebugGroup()
    {
        vgpuPopDebugGroup(Handle);
    }

    public  void InsertDebugMarker(string debugLabel)
    {
        fixed (sbyte* pLabel = debugLabel.GetUtf8Span())
        {
            vgpuInsertDebugMarker(Handle, pLabel);
        }
    }

    public Texture? AcquireSwapchainTexture(SwapChain swapChain)
    {
        
        VGPUTexture textureHandle = vgpuAcquireSwapchainTexture(Handle, swapChain.Handle);
        if (textureHandle.IsNull)
            return default;

        return new Texture(Device, textureHandle);
    }

    public void BeginRenderPass(Texture texture, in Vector4 clearColor)
    {
        Guard.IsNotNull(texture, nameof(texture));

        VGPURenderPassColorAttachment colorAttachment = new()
        {
            texture = texture.Handle,
            loadAction = VGPULoadAction.Clear,
            storeAction = VGPUStoreAction.Store,
            clearColor = new VGPUColor()
            {
                r = clearColor.X,
                g = clearColor.Y,
                b = clearColor.Z,
                a = clearColor.W
            }
        };

        VGPURenderPassDesc renderPassDesc = new()
        {
            colorAttachmentCount = 1u,
            colorAttachments = &colorAttachment
        };
        vgpuBeginRenderPass(Handle, &renderPassDesc);
    }

    public void EndRenderPass()
    {
        vgpuEndRenderPass(Handle);
    }

    public IDisposable PushScopedDebugGroup(string groupLabel)
    {
        PushDebugGroup(groupLabel);
        return new ScopedDebugGroup(this);
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
