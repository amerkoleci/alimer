// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Engine;

public sealed class CameraSystem : EntitySystem<CameraComponent>
{
    public CameraSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));

        // TODO: Handle multiple windows/viewport (not correct)
        MainWindow = services.GetService<Window>();
    }

    public Window MainWindow { get; }

    public override void Draw(RenderContext renderContext, Texture outputTexture, GameTime time)
    {
        foreach (CameraComponent cameraComponent in Components)
        {
            float screenAspectRatio = MainWindow.AspectRatio;

            cameraComponent.Update(screenAspectRatio);
        }
    }
}
