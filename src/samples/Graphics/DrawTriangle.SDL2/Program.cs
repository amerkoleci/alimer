// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice.Samples
{
    public static class Program
    {
        public static void Main(string[] args)
        {
            using DrawTriangleGame game = new(new SDL2GameContext());
            game.Run();
        }
    }
}
