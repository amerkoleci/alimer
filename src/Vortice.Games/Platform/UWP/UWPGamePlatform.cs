// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Vortice
{
    internal class UWPGamePlatform : GamePlatform
    {
        public UWPGamePlatform(Game game)
            : base(game)
        {
        }
    }

    internal partial class GamePlatform
    {
        public static GamePlatform Create(Game game)
        {
            return new UWPGamePlatform(game);
        }
    }
}
