// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public sealed class LightSystem : EntitySystem<LightComponent>
{
    public LightSystem(IServiceRegistry services)
        : base(typeof(TransformComponent))
    {
        ArgumentNullException.ThrowIfNull(services, nameof(services));
    }

    public override void Update(GameTime time)
    {
        foreach (LightComponent lightComponent in Components)
        {
        }
    }
}
