// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Microsoft.Extensions.DependencyInjection;

namespace Alimer
{
    public sealed class NetStandardGameContext : GameContext
    {
        public override GameWindow GameWindow => throw new NotImplementedException();
        
        public NetStandardGameContext()
        {
            int result = GLFW.glfwInit();
        }

        public override bool Run(Action loadAction, Action tickAction)
        {
            throw new NotImplementedException();
        }
    }
}
