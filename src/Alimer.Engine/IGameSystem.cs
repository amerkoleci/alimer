// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Engine;

public interface IGameSystem
{
    void Update(AppTime time);

    void BeginDraw();

    void Draw(AppTime time);

    void EndDraw();
}
