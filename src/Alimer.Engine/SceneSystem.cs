// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public sealed class SceneSystem : EntityManager
{
    private Entity? _rootEntity;

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
}
