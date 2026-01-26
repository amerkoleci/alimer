// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.ComponentModel;
using Alimer.Graphics;

namespace Alimer.Samples.Graphics;

[Description("Graphics - Hello Window")]
public sealed class HelloWindowSample : GraphicsSampleBase
{
    public HelloWindowSample(IServiceRegistry services, Window mainWindow)
        : base("Graphics - Hello Window", services, mainWindow)
    {
    }

    public override void Draw(CommandBuffer context, Texture swapChainTexture)
    {
        RenderPassColorAttachment colorAttachment = new(swapChainTexture.DefaultView!, new Color(0.3f, 0.3f, 0.3f));
        RenderPassDepthStencilAttachment depthStencilAttachment = new(DepthStencilTexture!);
        RenderPassDescriptor backBufferRenderPass = new(depthStencilAttachment, colorAttachment)
        {
            Label = "BackBuffer"u8
        };

        RenderPassEncoder renderPassEncoder = context.BeginRenderPass(backBufferRenderPass);
        renderPassEncoder.EndEncoding();
    }
}
