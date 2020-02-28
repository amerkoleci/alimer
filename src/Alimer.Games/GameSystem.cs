// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Threading.Tasks;

namespace Alimer
{
    /// <summary>
    /// Defines a base class for <see cref="Game"/> system.
    /// </summary>
    public abstract class GameSystem : IDisposable
    {
        public virtual void Dispose()
        {
        }

        public virtual void Initialize()
        {
        }

        public virtual Task LoadContentAsync()
        {
            return Task.CompletedTask;
        }

        public virtual void Update(GameTime gameTime)
        {
        }

        public virtual void BeginDraw()
        {
        }

        public virtual void Draw(GameTime gameTime)
        {
        }

        public virtual void EndDraw()
        {
        }
    }
}
