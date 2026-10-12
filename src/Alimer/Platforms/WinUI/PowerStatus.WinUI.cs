// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Windows.System.Power;

namespace Alimer;

partial class PowerStatus
{
    global::Windows.Devices.Power.Battery DefaultBattery =>
            global::Windows.Devices.Power.Battery.AggregateBattery;

    public static partial PowerLineStatus Status
    {
        get
        {
            
        }
    }

    public static partial BatteryStatus GetBatteryStatus(out int batteryLifeTime, out int batteryLifePercent)
    {
        SDL_PowerState state = SDL_GetPowerInfo(out batteryLifeTime, out batteryLifePercent);
        return state.FromSDL();
    }
}
