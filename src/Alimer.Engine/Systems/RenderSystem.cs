// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Engine;

public sealed class RenderSystem : EntitySystem<MeshComponent>
{
    public RenderSystem(GraphicsDevice graphicsDevice)
        : base(typeof(TransformComponent))
    {
        Guard.IsNotNull(graphicsDevice, nameof(GraphicsDevice));

        GraphicsDevice = graphicsDevice;
    }

    public GraphicsDevice GraphicsDevice { get; }

    public override void Draw(AppTime time)
    {
        RenderContext context = GraphicsDevice.BeginRenderContext("Render");
        //Texture? swapChainTexture = context.AcquireSwapChainTexture(MainWindow.SwapChain!);
        //if (swapChainTexture is not null)
        //{
        //    RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        //    RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        //    RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        //    {
        //        Label = "BackBuffer"
        //    };

        //    using (context.PushScopedPassPass(backBufferRenderPass))
        //    {
        //        foreach (MeshComponent meshComponent in Components)
        //        {
        //        }
        //    }
        //}

        //GraphicsDevice.Submit(commandBuffer);
        context.Flush(waitForCompletion: false);


    }
}
