// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;

namespace Vortice
{
    public abstract class GameContext
    {
        public virtual void ConfigureServices(IServiceCollection services)
        {
        }

        public abstract void RunMainLoop(Action callback);
    }
}
