// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Engine;

public sealed class SceneSystem : EntityManager
{
    private Entity? _rootEntity;

    public SceneSystem(GraphicsDevice graphicsDevice)
        : base(graphicsDevice)
    {
    }

    public CameraComponent? CurrentCamera { get; set; }

    public Entity? RootEntity
    {
        get => _rootEntity;
        set
        {
            if (_rootEntity == value)
                return;

            if (_rootEntity != null)
            {
                RemoveRoot(_rootEntity);
            }

            if (value != null)
            {
                AddRoot(value);
            }

            _rootEntity = value;
        }
    }

    protected override void AddComponent(EntityComponent component, Entity entity)
    {
        base.AddComponent(component, entity);

        // Assign first camera as current (if not specified)
        if (component is CameraComponent cameraComponent &&
            CurrentCamera is null)
        {
            CurrentCamera = cameraComponent;
        }
    }

    protected override void RemoveComponent(EntityComponent component, Entity entity)
    {
        base.RemoveComponent(component, entity);

        // Unassign first camera as current (if not specified)
        if (component is CameraComponent cameraComponent &&
            CurrentCamera == cameraComponent)
        {
            CurrentCamera = default;
        }
    }
}
