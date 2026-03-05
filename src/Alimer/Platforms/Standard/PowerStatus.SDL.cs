// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.SDL3;
using static Alimer.SDL3.SDL_PowerState;

namespace Alimer;

unsafe partial class PowerStatus
{
    public static partial PowerLineStatus Status
    {
        get
        {
            SDL_PowerState state = SDL_GetPowerInfo(null, null);
            return state switch
            {
                SDL_POWERSTATE_UNKNOWN => PowerLineStatus.Unknown,
                SDL_POWERSTATE_ON_BATTERY => PowerLineStatus.Offline,
                SDL_POWERSTATE_CHARGING => PowerLineStatus.Online,
                _ => PowerLineStatus.Unknown
            };
        }
    }

    public static partial BatteryStatus GetBatteryStatus(out int batteryLifeTime, out int batteryLifePercent)
    {
        int seconds, percent;
        SDL_PowerState state = SDL_GetPowerInfo(&seconds, &percent);
        batteryLifeTime = seconds;
        batteryLifePercent = percent;
        return state.FromSDL();
    }
}
