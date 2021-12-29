// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public abstract class GameSystem : IGameSystem
{
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
