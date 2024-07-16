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

    /// <inheritdoc />
    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
        }

        base.Dispose(disposing);
    }

    public override void Update(AppTime time)
    {
        base.Update(time);
    }
}
