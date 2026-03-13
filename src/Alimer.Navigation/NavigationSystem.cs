// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Engine;

namespace Alimer.Navigation;

public class NavigationSystem : EntitySystem<NavigationMeshComponent>
{
    public NavigationSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));
    }

    /// <summary>Finalizes an instance of the <see cref="NavigationSystem" /> class.</summary>
    ~NavigationSystem() => Dispose(disposing: false);

    protected override void Dispose(bool disposing)
    {
    }

    public override void Update(GameTime time)
    {
        base.Update(time);
    }
}
