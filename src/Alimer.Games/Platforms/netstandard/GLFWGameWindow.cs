// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Drawing;
using static Alimer.GLFW;

namespace Alimer
{
    internal class GLFWGameWindow : GameWindow
    {
        private readonly IntPtr _handle;

        public override RectangleF ClientBounds => throw new NotImplementedException();

        public override string Title { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }

        public GLFWGameWindow()
        {
            glfwWindowHint(WindowIntHint.ContextVersionMajor, 3);
            glfwWindowHint(WindowIntHint.ContextVersionMinor, 3);
            glfwWindowHint(WindowBoolHint.OpenGLForwardCompat, true);
            glfwWindowHint(OpenGLProfile.CoreProfile);
            _handle = glfwCreateWindow(800, 600, "Alimer", IntPtr.Zero, IntPtr.Zero);
        }

        public bool ShouldClose() => glfwWindowShouldClose(_handle);
        public void SwapBuffers() => glfwSwapBuffers(_handle);
    }
}
