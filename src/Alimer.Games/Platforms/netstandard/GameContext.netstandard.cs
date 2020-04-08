// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using static Alimer.GLFW;

namespace Alimer
{
    public sealed class NetStandardGameContext : GameContext
    {
        /// <inheritdoc/>
        public override GameWindow GameWindow { get; }

        private GLFWGameWindow GLFWWindow => (GLFWGameWindow)GameWindow;

        public NetStandardGameContext()
        {
            if (!glfwInit())
            {
                return;
            }

            GameWindow = new GLFWGameWindow();
        }

        public override bool Run(Action loadAction, Action tickAction)
        {
            while (!GLFWWindow.ShouldClose())
            {
                GLFWWindow.SwapBuffers();
                glfwPollEvents();
            }

            glfwTerminate();
            return true;
        }
    }
}
