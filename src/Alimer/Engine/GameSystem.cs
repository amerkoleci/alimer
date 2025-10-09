// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Engine;

public abstract class GameSystem : IGameSystem
{
    public virtual void Update(GameTime time)
    {
    }

    public virtual void BeginDraw()
    {
    }

    public virtual void Draw(RenderContext renderContext, Texture outputTexture, GameTime time)
    {
    }

    public virtual void EndDraw()
    {
    }
}
