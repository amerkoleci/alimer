// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Engine;

public interface IGameSystem
{
    void Update(AppTime time);

    void BeginDraw();

    void Draw(RenderContext renderContext, Texture outputTexture, AppTime time);

    void EndDraw();
}
