// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public interface IGame : IGameSystem
{
    IServiceProvider Services { get; }

    IList<IGameSystem> GameSystems { get; }

    bool IsRunning { get; }
}
