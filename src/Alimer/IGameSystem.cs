// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer;

public interface IGameSystem
{
    void Update(GameTime time);

    void BeginDraw();

    void Draw(CommandBuffer commandBuffer, Texture outputTexture, GameTime time);

    void EndDraw();
}
