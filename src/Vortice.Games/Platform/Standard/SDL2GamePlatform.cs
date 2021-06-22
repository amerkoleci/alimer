// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice
{
    internal class SDL2GamePlatform : GamePlatform
    {
        public SDL2GamePlatform(Game game)
            : base(game)
        {
        }
    }

    internal partial class GamePlatform
    {
        public static GamePlatform Create(Game game)
        {
            return new SDL2GamePlatform(game);
        }
    }
}
