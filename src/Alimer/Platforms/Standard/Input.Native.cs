// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Input;

internal class NativeInput : InputManager
{
    private readonly NativePlatform _platform;

    public NativeInput(NativePlatform platform)
    {
        _platform = platform;
    }
}
