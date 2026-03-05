// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using static Alimer.SDL3;
using static Alimer.SDL3.SDL_PowerState;

namespace Alimer;

internal static class SDLUtils
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static BatteryStatus FromSDL(this SDL_PowerState value)
    {
        return value switch
        {
            SDL_POWERSTATE_UNKNOWN => BatteryStatus.Unknown,
            SDL_POWERSTATE_NO_BATTERY => BatteryStatus.NotPresent,
            SDL_POWERSTATE_ON_BATTERY => BatteryStatus.OnBattery,
            SDL_POWERSTATE_CHARGING => BatteryStatus.Charging,
            SDL_POWERSTATE_CHARGED => BatteryStatus.Charged,
            _ => BatteryStatus.Unknown
        };
    }
}
