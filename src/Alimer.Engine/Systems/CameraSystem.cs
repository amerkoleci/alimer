// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Engine;

public sealed class CameraSystem : EntitySystem<CameraComponent>
{
    public CameraSystem()
        : base(typeof(TransformComponent))
    {
    }

    public GraphicsDevice? GraphicsDevice { get; set; }

    public override void Draw(RenderContext renderContext, Texture outputTexture, AppTime time)
    {
        foreach (CameraComponent cameraComponent in Components)
        {
            float? screenAspectRatio = null;

            //if (graphicsDevice.CommandList.Viewports.Length > 0)
            //{
            //    screenAspectRatio = graphicsDevice.CommandList.Viewports[0].Width / graphicsDevice.CommandList.Viewports[0].Height;
            //}

            cameraComponent.Update(screenAspectRatio);
        }
    }
}
