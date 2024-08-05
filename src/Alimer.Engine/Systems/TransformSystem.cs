// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public sealed class TransformSystem : EntitySystem<TransformComponent>
{
    private readonly HashSet<TransformComponent> _transformationRoots = [];

    public TransformSystem()
    {
    }

    public override void BeginDraw()
    {
        UpdateTransformations(_transformationRoots);
    }

    protected override void OnEntityComponentAdded(TransformComponent component)
    {
        if (component.Parent is null)
        {
            _transformationRoots.Add(component);
        }
    }

    protected override void OnEntityComponentRemoved(TransformComponent component)
    {
        _transformationRoots.Remove(component);
    }

    private static void UpdateTransformations(IEnumerable<TransformComponent> transformationRoots)
    {
        foreach (TransformComponent transformComponent in transformationRoots)
        {
            UpdateTransformationsRecursive(transformComponent);
        }
    }

    private static void UpdateTransformationsRecursive(TransformComponent transformComponent)
    {
        transformComponent.UpdateLocalMatrix();
        transformComponent.UpdateWorldMatrixInternal(false);

        foreach (TransformComponent child in transformComponent)
        {
            UpdateTransformationsRecursive(child);
        }
    }
}
