// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice;

public sealed class GameTime
{
    public GameTime()
    {
    }

    public TimeSpan Elapsed { get; private set; }

    public TimeSpan Total { get; private set; }

    internal void Update(TimeSpan totalTime, TimeSpan elapsedTime)
    {
        Total = totalTime;
        Elapsed = elapsedTime;
    }
}
