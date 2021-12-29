// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public interface IGameSystem
{
    void Update(GameTime gameTime);

    void BeginDraw();
    void Draw(GameTime gameTime);
    void EndDraw();
}
