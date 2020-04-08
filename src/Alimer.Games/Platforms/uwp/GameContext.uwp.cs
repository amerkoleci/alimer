﻿// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;

namespace Alimer
{
    /// <summary>
    /// Defines a context for <see cref="Game"/> that handles platform logic.
    /// </summary>
    public sealed class CoreWindowGameContext : GameContext
    {
        public override GameWindow GameWindow => throw new NotImplementedException();

        public override bool Run(Action loadAction, Action tickAction)
        {
            throw new NotImplementedException();
        }
    }
}
