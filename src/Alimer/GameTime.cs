// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer;

public class GameTime
{
    public GameTime()
    {
    }

    public GameTime(TimeSpan totalTime, TimeSpan elapsedTime)
    {
        Total = totalTime;
        Elapsed = elapsedTime;
    }

    internal void Update(TimeSpan totalTime, TimeSpan elapsedTime)
    {
        Total = totalTime;
        Elapsed = elapsedTime;
    }

    public TimeSpan Elapsed { get; private set; }

    public TimeSpan Total { get; private set; }
}
