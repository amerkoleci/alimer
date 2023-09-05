// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public abstract class GameSystem : IGameSystem
{
    public virtual void Update(AppTime time)
    {
    }

    public virtual void BeginDraw()
    {
    }

    public virtual void Draw(AppTime time)
    {
    }

    public virtual void EndDraw()
    {
    }
}
