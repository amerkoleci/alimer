// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace DrawTriangle
{
    public static class Program
    {
        public static void Main()
        {
            
            using (DrawTriangleGame game = new DrawTriangleGame())
            {
                game.Run();
            }
        }

    }
}
