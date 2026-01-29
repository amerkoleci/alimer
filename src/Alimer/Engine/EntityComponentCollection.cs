// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Collections.ObjectModel;

namespace Alimer.Engine;

public sealed class EntityComponentCollection(Entity entity) : ObservableCollection<Component>
{
    /// <summary>
    /// Gets the owner entity.
    /// </summary>
    public Entity Entity { get; } = entity;

    public void Add(params Component[] components)
    {
        foreach(Component component in components)
        {
            base.Add(component);
        }
    }

    protected override void InsertItem(int index, Component item)
    {
        if (Contains(item))
            return;

        TransformComponent oldTransformComponent = Entity.Transform;

        base.InsertItem(index, item);

        if (item is TransformComponent && item != oldTransformComponent)
        {
            Remove(oldTransformComponent);
        }
    }
}
