﻿// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public abstract class EntityComponent
{
    public Entity? Entity { get; internal set; }

    public virtual bool IsEnabled { get; set; } = true;
}
