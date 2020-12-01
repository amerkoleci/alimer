// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Alimer.Graphics;
using Microsoft.Extensions.DependencyInjection;
using static Alimer.GLFW;

namespace Alimer
{
    public class NetStandardGameContext : GameContext
    {
        /// <inheritdoc/>
        public override GameWindow? GameWindow { get; }

        private GLFWGameWindow? GLFWWindow => (GLFWGameWindow?)GameWindow;

        public NetStandardGameContext()
        {
            if (!glfwInit())
            {
                return;
            }

            glfwWindowHint(WindowClientApi.None);
            GameWindow = new GLFWGameWindow();
        }

        public override void ConfigureServices(IServiceCollection services)
        {
            base.ConfigureServices(services);
        }

        public override bool Run(Action loadAction, Action tickAction)
        {
            loadAction();

            while (!GLFWWindow!.ShouldClose())
            {
                tickAction();
                glfwPollEvents();
            }

            glfwTerminate();
            return true;
        }
    }
}
