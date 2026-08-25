// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.AlimerApi;

namespace Alimer;

partial class PowerStatus
{
    public static partial PowerLineStatus Status => alimerGetPowerLineStatus();

    public static partial BatteryStatus GetBatteryStatus(out int batteryLifeTime, out int batteryLifePercent)
    {
        return alimerGetBatteryStatus(out batteryLifeTime, out batteryLifePercent);
    }
}
