// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer
{
    /// <summary>
    /// Current timing used for variable-step (real time) or fixed-step (game time) games.
    /// </summary>
    public class GameTime
    {
        public GameTime()
        {
        }

        public GameTime(TimeSpan totalTime, TimeSpan elapsedTime)
        {
            Total = totalTime;
            Elapsed = elapsedTime;
        }

        internal void Update(TimeSpan totalTime, TimeSpan elapsedTime)
        {
            Total = totalTime;
            Elapsed = elapsedTime;
        }

        public TimeSpan Elapsed { get; private set; }

        public TimeSpan Total { get; private set; }
    }
}
