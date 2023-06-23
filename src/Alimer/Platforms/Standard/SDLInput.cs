// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

internal class SDLInput : InputManager
{
    private readonly SDLPlatform _platform;

    public SDLInput(SDLPlatform platform)
    {
        _platform = platform;
    }
}
