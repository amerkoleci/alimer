// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;

namespace Alimer.Navigation;

public class NavigationSystem : EntitySystem<NavigationMeshComponent>
{
    public NavigationSystem()
        : base(typeof(TransformComponent))
    {
    }

    /// <inheritdoc/>
    protected override void Destroy()
    {
    }

    public override void Update(GameTime time)
    {
        base.Update(time);
    }
}
