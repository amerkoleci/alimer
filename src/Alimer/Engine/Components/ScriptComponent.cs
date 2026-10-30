// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.Serialization;
using Alimer.Graphics;
using Alimer.Input;

namespace Alimer.Engine;

[EntitySystem(typeof(ScriptSystem))]
public abstract partial class ScriptComponent : Component
{
#nullable disable
    [IgnoreDataMember]
    public IServiceRegistry Services { get; private set; }
    [IgnoreDataMember]
    public GraphicsDevice GraphicsDevice { get; private set; }
    [IgnoreDataMember]
    public InputManager Input { get; private set; }
#nullable enable

    internal void Initialize(IServiceRegistry services)
    {
        Services = services;

        GraphicsDevice = services.GetRequiredService<GraphicsDevice>();
        Input = services.GetRequiredService<InputManager>();
    }

    public virtual void Cancel()
    {
    }
}
