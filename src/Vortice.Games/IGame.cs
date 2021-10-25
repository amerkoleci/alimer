// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using System.Collections.Generic;

namespace Vortice
{
    public interface IGame : IGameSystem
    {
        IServiceProvider Services { get; }

        IList<IGameSystem> GameSystems { get; }

        bool IsRunning { get; }

        void Initialize();

        void Run();
        void BeginRun();
        void EndRun();

        void Tick();
    }
}
