// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using Alimer.Graphics;
using Alimer.Numerics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Hello Window")]
public sealed class HelloWindowSample : GraphicsSampleBase
{
    public HelloWindowSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Hello Window", services, mainWindow)
    {
    }

    public override void Draw(RenderContext context, Texture swapChainTexture)
    {
        RenderPassColorAttachment colorAttachment = new(swapChainTexture, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescription backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"
        };

        using (context.PushScopedPassPass(backBufferRenderPass))
        {
        }
    }
}
