// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

namespace Alimer
{
    /// Identifiers the running platform type.
    enum class PlatformID
    {
        /// Windows platform.
        Windows,
        /// Linux platform.
        Linux,
        /// macOS platform.
        MacOS,
        /// Android platform.
        Android,
        /// iOS platform.
        iOS,
        /// Browser (WASM) platform.
        Browser,
        /// Xbox One platform.
        XboxOne,
        /// Xbox Series X|S platform.
        XboxScarlett,
    };

    /// Identifiers the running platform family.
    enum class PlatformFamily
    {
        /// Unknown family.
        Unknown,
        /// Mobile family.
        Mobile,
        /// Desktop family.
        Desktop,
        /// Console family.
        Console
    };

    enum class WindowFlags : uint32_t
    {
        None = 0,
        Fullscreen = 1 << 0,
        Hidden = 1 << 1,
        Borderless = 1 << 2,
        Resizable = 1 << 3,
        Maximized = 1 << 4,
        AlwaysOnTop = 1 << 5,
        Utility = 1 << 6,
        Tooltip = 1 << 7,
        PopupMenu = 1 << 8,
        Transparent = 1 << 9,
    };
    ALIMER_ENUM_CLASS_FLAG_OPERATORS(WindowFlags);

    constexpr static const int32_t WindowPositionCentered = Limits<int32_t>::Max;
}
