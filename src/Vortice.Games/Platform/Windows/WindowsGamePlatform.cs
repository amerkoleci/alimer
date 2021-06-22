// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Windows.Forms;

namespace Vortice
{
    internal class WindowsGamePlatform : GamePlatform
    {
        public WindowsGamePlatform(Game game)
            : base(game)
        {
        }
    }

    internal partial class GamePlatform
    {
        public static GamePlatform Create(Game game) => new WindowsGamePlatform(game);
    }
}
