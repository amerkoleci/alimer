// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace DrawTriangle
{
    public static class Program
    {
        public static void Main()
        {
            using (var game = new DrawTriangleGame())
            {
                game.Run();
            }
        }
    }
}
