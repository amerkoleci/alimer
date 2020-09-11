// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer
{
    /// <summary>
    /// Defines a context for <see cref="Game"/> that handles platform logic.
    /// </summary>
    internal class CoreWindowGameContext : GameContext
    {
        public override GameWindow GameWindow => throw new NotImplementedException();

        public CoreWindowGameContext(Game game)
            : base(game)
        {

        }

        public override bool Run(Action loadAction, Action tickAction)
        {
            throw new NotImplementedException();
        }
    }

    public partial class GameContext
    {
        public static GameContext Create(Game game) => new CoreWindowGameContext(game);
    }
}
